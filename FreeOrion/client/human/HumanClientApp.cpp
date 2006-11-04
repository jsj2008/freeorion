#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "HumanClientApp.h"

#include "../../UI/CUIControls.h"
#include "../../UI/CUIStyle.h"
#include "../../UI/MapWnd.h"
#include "../../network/Message.h"
#include "../../UI/MultiplayerLobbyWnd.h"
#include "../../util/MultiplayerCommon.h"
#include "../../util/OptionsDB.h"
#include "../../universe/Planet.h"
#include "../../util/Process.h"
#include "../../util/Serialize.h"
#include "../../util/SitRepEntry.h"
#include "../../util/Directories.h"
#include "../../util/XMLDoc.h"
#include "../../util/Version.h"
#include "../../Empire/Empire.h"

#include <GG/BrowseInfoWnd.h>

#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/FileAppender.hh>

#include <boost/lexical_cast.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>
#include <boost/serialization/vector.hpp>

#include <sstream>

#ifdef ENABLE_CRASH_BACKTRACE
# include <signal.h>
# include <execinfo.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <unistd.h>

void SigHandler(int sig)
{
    void* backtrace_buffer[100];
    int num;
    int fd;

    signal(sig, SIG_DFL);
    fd = open("crash.txt",O_WRONLY|O_CREAT|O_APPEND|O_SYNC,0666);
    if (fd != -1) {
        write(fd, "--- New crash backtrace begins here ---\n", 24);
        num = backtrace(backtrace_buffer, 100);
        backtrace_symbols_fd(backtrace_buffer, num, fd);
        backtrace_symbols_fd(backtrace_buffer, num, 2);
        close(fd);
    }

    // Now we try to display a MessageBox; this might fail and also
    // corrupt the heap, but since we're dying anyway that's no big
    // deal

    ClientUI::MessageBox("The client has just crashed!\nFile a bug report and\nattach the file called 'crash.txt'\nif necessary", true);

    // Try SDL-shutdown
    SDL_Quit();
    raise(sig);
}
#endif //ENABLE_CRASH_BACKTRACE

namespace {
    // command-line options
    void AddOptions(OptionsDB& db)
    {
        db.Add("autosave.single-player", "OPTIONS_DB_AUTOSAVE_SINGLE_PLAYER", true, Validator<bool>());
        db.Add("autosave.multiplayer", "OPTIONS_DB_AUTOSAVE_MULTIPLAYER", false, Validator<bool>());
        db.Add("autosave.turns", "OPTIONS_DB_AUTOSAVE_TURNS", 5, RangedValidator<int>(1, 50));
        db.Add("autosave.saves", "OPTIONS_DB_AUTOSAVE_SAVES", 10, RangedValidator<int>(1, 50));
#if defined(FREEORION_LINUX)
        db.Add("enable-sdl-event-thread", "OPTIONS_DB_ENABLE_SDL_EVENT_THREAD", false, Validator<bool>());
#endif
        db.Add("music-volume", "OPTIONS_DB_MUSIC_VOLUME", 255, RangedValidator<int>(1, 255));
    }
    bool temp_bool = RegisterOptions(&AddOptions);

}
 
HumanClientApp::HumanClientApp() : 
    ClientApp(), 
    SDLGUI(GetOptionsDB().Get<int>("app-width"), 
           GetOptionsDB().Get<int>("app-height"),
           false, "freeorion"),
    m_single_player_game(true),
    m_game_started(false),
    m_turns_since_autosave(0),
    m_handling_message(false)
{
#ifdef ENABLE_CRASH_BACKTRACE
    signal(SIGSEGV, SigHandler);
#endif

    const std::string LOG_FILENAME((GetLocalDir() / "freeorion.log").native_file_string());

    // a platform-independent way to erase the old log We cannot use
    // boost::filesystem::ofstream here, as stupid b::f won't allow us
    // to have a dot in the directory name, which is where local data
    // is kept under unix.
    std::ofstream temp(LOG_FILENAME.c_str());
    temp.close();

    log4cpp::Appender* appender = new log4cpp::FileAppender("FileAppender", LOG_FILENAME);
    log4cpp::PatternLayout* layout = new log4cpp::PatternLayout();
    layout->setConversionPattern("%d %p Client : %m%n");
    appender->setLayout(layout);
    Logger().setAdditivity(false);  // make appender the only appender used...
    Logger().setAppender(appender);
    Logger().setAdditivity(true);   // ...but allow the addition of others later
    Logger().setPriority(PriorityValue(GetOptionsDB().Get<std::string>("log-level")));

    SetMaxFPS(60.0);

    boost::shared_ptr<GG::StyleFactory> style(new CUIStyle());
    SetStyleFactory(style);

    GUI::SetMinDragTime(0);
}

HumanClientApp::~HumanClientApp()
{
}

Message HumanClientApp::TurnOrdersMessage(bool save_game_data/* = false*/) const
{
    if (save_game_data) {
        std::ostringstream os;
        {
            boost::archive::xml_oarchive oa(os);
            Serialize(&oa, m_orders);
            bool ui_data_available = true;
            oa << BOOST_SERIALIZATION_NVP(ui_data_available);
            SaveGameUIData ui_data;
            ClientUI::GetClientUI()->GetSaveGameUIData(ui_data);
            oa << BOOST_SERIALIZATION_NVP(ui_data);
        }
        return ClientSaveDataMessage(m_player_id, os.str());
    } else {
        std::ostringstream os;
        {
            boost::archive::xml_oarchive oa(os);
            Serialize(&oa, m_orders);
        }
        return ::TurnOrdersMessage(m_player_id, os.str());
    }
}

std::map<int, int> HumanClientApp::PendingColonizationOrders() const
{
    std::map<int, int> retval;
    for (OrderSet::const_iterator it = m_orders.begin(); it != m_orders.end(); ++it) {
        if (const FleetColonizeOrder* order = dynamic_cast<const FleetColonizeOrder*>(it->second)) {
            retval[order->PlanetID()] = it->first;
        }
    }
    return retval;
}

void HumanClientApp::StartServer()
{
#ifdef FREEORION_WIN32
    const std::string SERVER_CLIENT_EXE = "freeoriond.exe";
#else
    const std::string SERVER_CLIENT_EXE = (GetBinDir() / "freeoriond").native_file_string();
#endif
    std::vector<std::string> args(1, SERVER_CLIENT_EXE);
    args.push_back("--settings-dir");
    args.push_back("\"" + GetOptionsDB().Get<std::string>("settings-dir") + "\"");
    args.push_back("--log-level");
    args.push_back(GetOptionsDB().Get<std::string>("log-level"));
    m_server_process = Process(SERVER_CLIENT_EXE, args);
}

void HumanClientApp::FreeServer()
{
    m_server_process.Free();
    m_player_id = -1;
    m_empire_id = -1;
    m_player_name = "";
}

void HumanClientApp::KillServer()
{
    m_server_process.Kill();
    m_player_id = -1;
    m_empire_id = -1;
    m_player_name = "";
}

void HumanClientApp::EndGame()
{
    m_game_started = false;
    m_network_core.DisconnectFromServer();
    m_server_process.RequestTermination();
    m_player_id = -1;
    m_empire_id = -1;
    m_player_name = "";
    m_ui->GetMapWnd()->Sanitize();
    m_ui->ScreenIntro();
}

void HumanClientApp::SetLobby(MultiplayerLobbyWnd* lobby)
{
    m_multiplayer_lobby_wnd = lobby;
}

bool HumanClientApp::LoadSinglePlayerGame()
{
    std::vector<std::pair<std::string, std::string> > save_file_types;
    save_file_types.push_back(std::pair<std::string, std::string>(UserString("GAME_MENU_SAVE_FILES"), "*.sav"));

    try {
        FileDlg dlg(GetOptionsDB().Get<std::string>("save-dir"), "", false, false, save_file_types);
        dlg.Run();
        std::string filename;
        if (!dlg.Result().empty()) {
            filename = *dlg.Result().begin();

            if (!NetworkCore().Connected()) {
                if (!GetOptionsDB().Get<bool>("force-external-server"))
                    StartServer();

                bool failed = false;
                int start_time = Ticks();
                const int SERVER_CONNECT_TIMEOUT = 30000; // in ms
                while (!NetworkCore().ConnectToLocalhostServer()) {
                    if (SERVER_CONNECT_TIMEOUT < Ticks() - start_time) {
                        ClientUI::MessageBox(UserString("ERR_CONNECT_TIMED_OUT"), true);
                        failed = true;
                        break;
                    }
                }

                if (failed) {
                    KillServer();
                    return false;
                }
            }

            m_ui->ScreenLoad();
            m_game_started = false;
            m_player_id = NetworkCore::HOST_PLAYER_ID;
            m_empire_id = -1;
            m_player_name = "Happy_Player";

            // HACK!  send the multiplayer form of the HostGameMessage, since it establishes us as the host, and the single-player 
            // LOAD_GAME message will establish us as a single-player game
            XMLDoc parameters;
            parameters.root_node.AppendChild(XMLElement("host_player_name", std::string("Happy_Player")));
            NetworkCore().SendMessage(HostGameMessage(NetworkCore::HOST_PLAYER_ID, parameters));
            NetworkCore().SendMessage(HostLoadGameMessage(NetworkCore::HOST_PLAYER_ID, filename));

            return true;
        }
    } catch (const FileDlg::BadInitialDirectory& e) {
        ClientUI::MessageBox(e.what(), true);
    }
    return false;
}

void HumanClientApp::Enter2DMode()
{
    glPushAttrib(GL_ENABLE_BIT | GL_PIXEL_MODE_BIT | GL_TEXTURE_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, AppWidth(), AppHeight()); //removed -1 from AppWidth & Height

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    // set up coordinates with origin in upper-left and +x and +y directions right and down, respectively
    // the depth of the viewing volume is only 1 (from 0.0 to 1.0)
    glOrtho(0.0, AppWidth(), AppHeight(), 0.0, 0.0, AppWidth());

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void HumanClientApp::Exit2DMode()
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glPopAttrib();
}

log4cpp::Category& HumanClientApp::Logger()
{
    return log4cpp::Category::getRoot();
}

HumanClientApp* HumanClientApp::GetApp()
{
    return dynamic_cast<HumanClientApp*>(GG::GUI::GetGUI());
}

void HumanClientApp::StartTurn()
{
    // setup GUI
    m_ui->ScreenProcessTurn();

    // call base method
    ClientApp::StartTurn();
}

void HumanClientApp::SDLInit()
{
    const SDL_VideoInfo* vid_info = 0;
    Uint32 DoFullScreen = 0;

    // Set Fullscreen if specified at command line or in config-file
    DoFullScreen = GetOptionsDB().Get<bool>("fullscreen") ? SDL_FULLSCREEN : 0;

    // SDL on MacOsX crashes if the Eventhandling-thread isn't the
    // application's main thread. It seems that only the applications
    // main-thread is able to receive events...
#if defined(FREEORION_WIN32) || defined(FREEORION_MACOSX) 
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) < 0) {
#else
    Uint32 init_flags = SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE;
    if (GetOptionsDB().Get<bool>("enable-sdl-event-thread"))
        init_flags |= SDL_INIT_EVENTTHREAD;
    if (SDL_Init(init_flags) < 0) {
#endif
        Logger().errorStream() << "SDL initialization failed: " << SDL_GetError();
        Exit(1);
    }

    SDL_WM_SetCaption(("FreeOrion " + FreeOrionVersionString()).c_str(), "FreeOrion");

    if (SDLNet_Init() < 0) {
        Logger().errorStream() << "SDL Net initialization failed: " << SDLNet_GetError();
        Exit(1);
    }

    if (FE_Init() < 0) {
        Logger().errorStream() << "FastEvents initialization failed: " << FE_GetError();
        Exit(1);
    }

    vid_info = SDL_GetVideoInfo();

    if (!vid_info) {
        Logger().errorStream() << "Video info query failed: " << SDL_GetError();
        Exit(1);
    }

    int bpp = boost::lexical_cast<int>(GetOptionsDB().Get<int>("color-depth"));
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    if (24 <= bpp) {
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    } else { // assumes 16 bpp minimum
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    }

    if (SDL_SetVideoMode(AppWidth(), AppHeight(), bpp, DoFullScreen | SDL_OPENGL) == 0) {
        Logger().errorStream() << "Video mode set failed: " << SDL_GetError();
        Exit(1);
    }

    if (NET2_Init() < 0) {
        Logger().errorStream() << "SDL Net2 initialization failed: " << NET2_GetError();
        Exit(1);
    }

    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
    EnableMouseButtonDownRepeat(SDL_DEFAULT_REPEAT_DELAY / 2, SDL_DEFAULT_REPEAT_INTERVAL / 2);

    Logger().debugStream() << "SDLInit() complete.";
    GLInit();
}

void HumanClientApp::GLInit()
{
    double ratio = AppWidth() / (float)(AppHeight());

    glEnable(GL_BLEND);
    glClearColor(0, 0, 0, 0);
    glViewport(0, 0, AppWidth(), AppHeight());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50.0, ratio, 0.0, 10.0);
    gluLookAt(0.0, 0.0, 5.0, 
              0.0, 0.0, 0.0, 
              0.0, 1.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    SDL_GL_SwapBuffers();
    Logger().debugStream() << "GLInit() complete.";
}

void HumanClientApp::Initialize()
{
    m_ui = boost::shared_ptr<ClientUI>(new ClientUI());
    m_ui->ScreenIntro(); // start the first screen; the UI takes over from there.

    if (!(GetOptionsDB().Get<bool>("music-off")))
        PlayMusic(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("bg-music"), -1);

    SetMusicVolume(GetOptionsDB().Get<int>("music-volume"));
    SetUISoundsVolume(GetOptionsDB().Get<int>("UI.sound.volume"));

    boost::shared_ptr<GG::BrowseInfoWnd> default_browse_info_wnd(
        new GG::TextBoxBrowseInfoWnd(400, GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()),
                                     GG::Clr(0, 0, 0, 200), ClientUI::WndOuterBorderColor(), ClientUI::TextColor(),
                                     GG::TF_LEFT | GG::TF_WORDBREAK, 1));
    GG::Wnd::SetDefaultBrowseInfoWnd(default_browse_info_wnd);
}

void HumanClientApp::HandleSystemEvents(int& last_mouse_event_time)
{
    // handle events
    SDL_Event event;
    while (0 < (m_handling_message ?
                FE_PollMaskedEvent(&event, SDL_ALLEVENTS & ~SDL_EVENTMASK(SDL_USEREVENT)) :
                FE_PollEvent(&event))) {
        if (event.type  == SDL_MOUSEBUTTONDOWN || event.type  == SDL_MOUSEBUTTONUP || event.type == SDL_MOUSEMOTION)
            last_mouse_event_time = Ticks();

        bool send_to_gg = false;
        EventType gg_event = MOUSEMOVE;
        GG::Key key = GGKeyFromSDLKey(event.key.keysym);
        Uint32 key_mods = SDL_GetModState();
        GG::Pt mouse_pos(event.motion.x, event.motion.y);
        GG::Pt mouse_rel(event.motion.xrel, event.motion.yrel);

        switch (event.type) {
        case SDL_KEYDOWN:
            if (key < GG::GGK_NUMLOCK)
                send_to_gg = true;
            gg_event = KEYPRESS;
            break;
        case SDL_MOUSEMOTION:
            send_to_gg = true;
            gg_event = MOUSEMOVE;
            break;
        case SDL_MOUSEBUTTONDOWN:
            send_to_gg = true;
            switch (event.button.button) {
                case SDL_BUTTON_LEFT:      gg_event = LPRESS; break;
                case SDL_BUTTON_MIDDLE:    gg_event = MPRESS; break;
                case SDL_BUTTON_RIGHT:     gg_event = RPRESS; break;
                case SDL_BUTTON_WHEELUP:   gg_event = MOUSEWHEEL; mouse_rel = GG::Pt(0, 1); break;
                case SDL_BUTTON_WHEELDOWN: gg_event = MOUSEWHEEL; mouse_rel = GG::Pt(0, -1); break;
            }
            key_mods = SDL_GetModState();
            break;
        case SDL_MOUSEBUTTONUP:
            send_to_gg = true;
            switch (event.button.button) {
                case SDL_BUTTON_LEFT:   gg_event = LRELEASE; break;
                case SDL_BUTTON_MIDDLE: gg_event = MRELEASE; break;
                case SDL_BUTTON_RIGHT:  gg_event = RRELEASE; break;
            }
            key_mods = SDL_GetModState();
            break;
        }

        if (send_to_gg)
            HandleGGEvent(gg_event, key, key_mods, mouse_pos, mouse_rel);
        else
            HandleNonGGEvent(event);
    }
}

void HumanClientApp::HandleNonGGEvent(const SDL_Event& event)
{
    switch(event.type) {
    case SDL_USEREVENT: {
        int net2_type = NET2_GetEventType(const_cast<SDL_Event*>(&event));
        if (net2_type == NET2_ERROREVENT || 
            net2_type == NET2_TCPACCEPTEVENT || 
            net2_type == NET2_TCPRECEIVEEVENT || 
            net2_type == NET2_TCPCLOSEEVENT || 
            net2_type == NET2_UDPRECEIVEEVENT)
            m_network_core.HandleNetEvent(const_cast<SDL_Event&>(event));
        break;
    }

    case SDL_QUIT: {
        Exit(0);
        return;
    }
    }
}

void HumanClientApp::RenderBegin()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // this is the only line in SDLGUI::RenderBegin()
}

void HumanClientApp::FinalCleanup()
{
    if (NetworkCore().Connected()) {
        NetworkCore().DisconnectFromServer();
    }
    m_server_process.RequestTermination();
}

void HumanClientApp::SDLQuit()
{
    FinalCleanup();
    NET2_Quit();
    FE_Quit();
    SDLNet_Quit();
    SDL_Quit();
    Logger().debugStream() << "SDLQuit() complete.";
}

void HumanClientApp::HandleMessageImpl(const Message& msg)
{
    m_handling_message = true;
    switch (msg.Type()) {
    case Message::SERVER_STATUS: {
        std::stringstream stream(msg.GetText());
        XMLDoc doc;
        doc.ReadDoc(stream);
        if (doc.root_node.ContainsChild("new_name")) {
            m_player_name = doc.root_node.Child("new_name").Text();
            Logger().debugStream() << "HumanClientApp::HandleMessageImpl : Received SERVER_STATUS -- Server has renamed this player \"" << 
                m_player_name  << "\"";
        } else if (doc.root_node.ContainsChild("server_state")) {
            ServerState server_state = ServerState(boost::lexical_cast<int>(doc.root_node.Child("server_state").Attribute("value")));
            Logger().debugStream() << "HumanClientApp::HandleMessageImpl : Received SERVER_STATUS (status code " << 
                doc.root_node.Child("server_state").Attribute("value") << ")";
            if (server_state == SERVER_DYING)
                KillServer();
        } else if (doc.root_node.ContainsChild("settings_files")) {
            std::string settings_files = doc.root_node.Child("settings_files").Text();
            std::string source_files = doc.root_node.Child("source_files").Text();
            Logger().debugStream() << "HumanClientApp::HandleMessageImpl : Received SERVER_STATUS -- Connection rejected by server, "
                "because different versions of the following settings and/or source files are in use by the client and the server: " << 
                settings_files << " " << source_files;
            ClientUI::MessageBox(UserString("ERR_VERSION_MISMATCH") + settings_files + " " + source_files, true);
            EndGame();
        }
        break;
    } 

    case Message::HOST_GAME: {
        if (msg.Sender() == -1 && msg.GetText() == "ACK")
            Logger().debugStream() << "HumanClientApp::HandleMessageImpl : Received HOST_GAME acknowledgement";
        break;
    } 

    case Message::JOIN_GAME: {
        if (msg.Sender() == -1) {
            if (m_player_id == -1) {
                m_player_id = boost::lexical_cast<int>(msg.GetText());
                Logger().debugStream() << "HumanClientApp::HandleMessageImpl : Received JOIN_GAME acknowledgement "
                    "(joined as player " << m_player_id << ")";
            } else if (m_player_id != NetworkCore::HOST_PLAYER_ID) {
                Logger().errorStream() << "HumanClientApp::HandleMessageImpl : Received erroneous JOIN_GAME acknowledgement when "
                    "already in a game";
            }
        }
        break;
    }

    case Message::GAME_START: {
        if (msg.Sender() == -1) {
            Logger().debugStream() << "HumanClientApp::HandleMessageImpl : Received GAME_START message; "
                "starting player turn...";
            m_game_started = true;
            std::istringstream is(msg.GetText());
            boost::archive::xml_iarchive ia(is);
            ia >> boost::serialization::make_nvp("single_player_game", m_single_player_game);
            ia >> boost::serialization::make_nvp("empire_id", m_empire_id);
            ia >> BOOST_SERIALIZATION_NVP(m_current_turn);
            Universe::s_encoding_empire = m_empire_id;
            Deserialize(&ia, Empires());
            Deserialize(&ia, GetUniverse());

            Orders().Reset();

            Logger().debugStream() << "HumanClientApp::HandleMessageImpl : Universe setup complete.";

            for (Empire::SitRepItr it = Empires().Lookup(m_empire_id)->SitRepBegin(); it != Empires().Lookup(m_empire_id)->SitRepEnd(); ++it) {
                m_ui->GenerateSitRepText(*it);
            }

            Autosave(true);
            m_ui->ScreenMap();
            m_ui->InitTurn(m_current_turn); // init the new turn
        }
        break;
    }

    case Message::SAVE_GAME: {
        NetworkCore().SendMessage(TurnOrdersMessage(true));
        break;
    }

    case Message::LOAD_GAME: {
        std::istringstream is(msg.GetText());
        boost::archive::xml_iarchive ia(is);
        bool ui_data_available;
        SaveGameUIData ui_data;
        Deserialize(&ia, Orders());
        Orders().ApplyOrders();
        ia >> BOOST_SERIALIZATION_NVP(ui_data_available);
        if (ui_data_available) {
            ia >> BOOST_SERIALIZATION_NVP(ui_data);
            ClientUI::GetClientUI()->RestoreFromSaveData(ui_data);
        }
        break;
    }

    case Message::TURN_UPDATE: {
        std::istringstream is(msg.GetText());
        boost::archive::xml_iarchive ia(is);
        Universe::s_encoding_empire = m_empire_id;
        ia >> BOOST_SERIALIZATION_NVP(m_current_turn);
        Deserialize(&ia, Empires());
        Deserialize(&ia, GetUniverse());

        // Now decode sitreps
        // Empire sitreps need UI in order to generate text, since it needs string resources
        // generate textr for all sitreps
        for (Empire::SitRepItr sitrep_it = Empires().Lookup(m_empire_id)->SitRepBegin(); sitrep_it != Empires().Lookup( m_empire_id )->SitRepEnd(); ++sitrep_it) {
            SitRepEntry *pEntry = *sitrep_it;
            m_ui->GenerateSitRepText(pEntry);
        }

        Autosave(false);

        // if this is the last turn, the TCP message handling inherent in Autosave()'s synchronous message may have
        // processed an end-of-game message, in which case we need *not* to execute these last two lines below
        if (!m_game_started || !NetworkCore().Connected())
            break;

        m_ui->ScreenMap(); 
        m_ui->InitTurn(m_current_turn);
        break;
    }

    case Message::TURN_PROGRESS: {
        XMLDoc doc;
        int phase_id;
        int empire_id;
        std::string phase_str;
        std::stringstream stream(msg.GetText());

        doc.ReadDoc(stream);          

        phase_id = boost::lexical_cast<int>(doc.root_node.Child("phase_id").Attribute("value"));
        empire_id = boost::lexical_cast<int>(doc.root_node.Child("empire_id").Attribute("value"));

        // given IDs, build message
        if (phase_id == Message::FLEET_MOVEMENT)
            phase_str = UserString("TURN_PROGRESS_PHASE_FLEET_MOVEMENT");
        else if (phase_id == Message::COMBAT)
            phase_str = UserString("TURN_PROGRESS_PHASE_COMBAT");
        else if (phase_id == Message::EMPIRE_PRODUCTION)
            phase_str = UserString("TURN_PROGRESS_PHASE_EMPIRE_GROWTH");
        else if (phase_id == Message::WAITING_FOR_PLAYERS)
            phase_str = UserString("TURN_PROGRESS_PHASE_WAITING");
        else if (phase_id == Message::PROCESSING_ORDERS)
            phase_str = UserString("TURN_PROGRESS_PHASE_ORDERS");
        else if (phase_id == Message::DOWNLOADING)
            phase_str = UserString("TURN_PROGRESS_PHASE_DOWNLOADING");

        m_ui->UpdateTurnProgress( phase_str, empire_id);
        break;
    }

    case Message::COMBAT_START:
    case Message::COMBAT_ROUND_UPDATE:
    case Message::COMBAT_END:{
        m_ui->UpdateCombatTurnProgress(msg.GetText());
        break;
    }

    case Message::HUMAN_PLAYER_MSG: {
        ClientUI::GetClientUI()->GetMapWnd()->HandlePlayerChatMessage(msg.GetText());
        break;
    }

    case Message::PLAYER_ELIMINATED: {
        Logger().debugStream() << "HumanClientApp::HandleMessageImpl : Message::PLAYER_ELIMINATED : m_empire_id=" << m_empire_id << " Empires().Lookup(m_empire_id)=" << Empires().Lookup(m_empire_id);
        Empire* empire = Empires().Lookup(m_empire_id);
        if (!empire) break;
        if (empire->Name() == msg.GetText()) {
            // TODO: replace this with something better
            ClientUI::MessageBox(UserString("PLAYER_DEFEATED"));
            EndGame();
        } else {
            // TODO: replace this with something better
            ClientUI::MessageBox(boost::io::str(boost::format(UserString("EMPIRE_DEFEATED")) % msg.GetText()));
        }
        break;
    }

    case Message::PLAYER_EXIT: {
        std::string message = boost::io::str(boost::format(UserString("PLAYER_DISCONNECTED")) % msg.GetText());
        ClientUI::MessageBox(message, true);
        break;
    }

    case Message::END_GAME: {
        if (m_game_started) {
            if (msg.GetText() == "VICTORY") {
                EndGame();
                // TODO: replace this with something better
                ClientUI::MessageBox(UserString("PLAYER_VICTORIOUS"));
            } else {
                EndGame();
                ClientUI::MessageBox(UserString("SERVER_GAME_END"));
            }
        }
        break;
    }

    default: {
        Logger().errorStream() << "HumanClientApp::HandleMessageImpl : Received unknown Message type code " << msg.Type();
        break;
    }
    }
    m_handling_message = false;
}

void HumanClientApp::HandleServerDisconnectImpl()
{
    if (m_multiplayer_lobby_wnd) { // in MP lobby
        ClientUI::MessageBox(UserString("MPLOBBY_HOST_ABORTED_GAME"), true);
        m_multiplayer_lobby_wnd->Cancel();
    } else if (m_game_started) { // playing game
        ClientUI::MessageBox(UserString("SERVER_LOST"), true);
        EndGame();
    }
}

void HumanClientApp::Autosave(bool new_game)
{
    if (((m_single_player_game && GetOptionsDB().Get<bool>("autosave.single-player")) || 
         (!m_single_player_game && GetOptionsDB().Get<bool>("autosave.multiplayer"))) &&
        (m_turns_since_autosave++ % GetOptionsDB().Get<int>("autosave.turns")) == 0) {
        const char* legal_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";
        std::string empire_name = Empires().Lookup(EmpireID())->Name();
        std::string::size_type first_good_empire_char = empire_name.find_first_of(legal_chars);
        if (first_good_empire_char == std::string::npos) {
            empire_name = "";
        } else {
            std::string::size_type first_bad_empire_char = empire_name.find_first_not_of(legal_chars, first_good_empire_char);
            empire_name = empire_name.substr(first_good_empire_char, first_bad_empire_char - first_good_empire_char);
        }

        std::string save_filename;
        if (m_single_player_game) {
            save_filename = boost::io::str(boost::format("AS_%s_%04d.sav") % empire_name % m_current_turn);
        } else {
            std::string::size_type first_good_player_char = m_player_name.find_first_of(legal_chars);
            if (first_good_player_char == std::string::npos) {
                save_filename = boost::io::str(boost::format("AS_%s_%04d.mps") % empire_name % m_current_turn);
            } else {
                std::string::size_type first_bad_player_char = m_player_name.find_first_not_of(legal_chars, first_good_player_char);
                std::string player_name = m_player_name.substr(first_good_player_char, first_bad_player_char - first_good_player_char);
                save_filename = boost::io::str(boost::format("AS_%s_%s_%04d.mps") % player_name % empire_name % m_current_turn);
            }
        }

        std::set<std::string> similar_save_files;
        std::set<std::string> old_save_files;
        std::string extension = m_single_player_game ? ".sav" : ".mps";
        namespace fs = boost::filesystem;
        fs::path save_dir(GetOptionsDB().Get<std::string>("save-dir"));
        fs::directory_iterator end_it;
        for (fs::directory_iterator it(save_dir); it != end_it; ++it) {
            if (!fs::is_directory(*it)) {
                std::string filename = it->leaf();
                if (!new_game &&
                    filename.find(extension) == filename.size() - extension.size() && 
                    filename.find(save_filename.substr(0, save_filename.size() - 7)) == 0) {
                    similar_save_files.insert(filename);
                } else if (filename.find("AS_") == 0) {
                    // this simple condition means that at the beginning of an autosave run, we'll clear out all old autosave files,
                    // even if they don't match the current empire name, or if they are MP vs. SP games, or whatever
                    old_save_files.insert(filename);
                }
            }
        }

        for (std::set<std::string>::iterator it = old_save_files.begin(); it != old_save_files.end(); ++it) {
            fs::remove(save_dir / *it);
        }

        unsigned int max_autosaves = GetOptionsDB().Get<int>("autosave.saves");
        std::set<std::string>::reverse_iterator rit = similar_save_files.rbegin();
        std::advance(rit, std::min(similar_save_files.size(), (size_t)(max_autosaves - 1)));
        for (; rit != similar_save_files.rend(); ++rit) {
            fs::remove(save_dir / *rit);
        }

        Message response;
        bool save_succeeded = 
            NetworkCore().SendSynchronousMessage(HostSaveGameMessage(PlayerID(), (save_dir / save_filename).native_file_string()), response);
        if (!save_succeeded && m_game_started && NetworkCore().Connected())
            Logger().errorStream() << "HumanClientApp::Autosave : An error occured while attempting to save the autosave file \"" << save_filename << "\"";
    }
}

/* Default sound implementation, do nothing */
void HumanClientApp::PlayMusic(const boost::filesystem::path& path, int loops /* = 0*/)
{}

void HumanClientApp::StopMusic()
{}

void HumanClientApp::PlaySound(const boost::filesystem::path& path)
{}

void HumanClientApp::FreeSound(const boost::filesystem::path& path)
{}

void HumanClientApp::FreeAllSounds()
{}

void HumanClientApp::SetMusicVolume(int vol)
{}

void HumanClientApp::SetUISoundsVolume(int vol)
{}
