import os
import re
import sys
import distutils.sysconfig
import distutils.util

gPreBuildVariants = [
    {
        'name': 'main',
    },
]


def main(all=False):
    basepath = os.path.dirname(__file__)
    builddir = os.path.join(basepath, 'prebuilt')
    if not os.path.exists(builddir):
        os.makedirs(builddir)
    src = os.path.join(basepath, 'src', 'main.c')

    cfg = distutils.sysconfig.get_config_vars()

    BASE_CFLAGS = cfg['CFLAGS']
    BASE_CFLAGS = BASE_CFLAGS.replace('-dynamic', '')
    while True:
        x = re.sub('-arch\s+\S+', '', BASE_CFLAGS)
        if x == BASE_CFLAGS:
            break
        BASE_CFLAGS=x

    while True:
        x = re.sub('-isysroot\s+\S+', '', BASE_CFLAGS)
        if x == BASE_CFLAGS:
            break
        BASE_CFLAGS=x

    arch = distutils.util.get_platform().split('-')[-1]
    if sys.prefix.startswith('/System') and \
            sys.version_info[:2] == (2,5):
        arch = "fat"

    name = 'main-' + arch

    for entry in gPreBuildVariants:
        if (not all) and entry['name'] != name: continue

        CC=cfg['CC']
        CFLAGS = BASE_CFLAGS + ' ' + cfg['ARCHFLAGS'] + ' -fPIE -pie'
        dest = os.path.join(builddir, entry['name'])
        if not os.path.exists(dest) or (
                os.stat(dest).st_mtime < os.stat(src).st_mtime):
            os.system('"%(CC)s" -o "%(dest)s" "%(src)s" %(CFLAGS)s' % locals())
            os.system('strip -Sx "%(dest)s"' % locals())

    dest = os.path.join(
            builddir,
            'main'
    )

    return dest


if __name__ == '__main__':
    main(all=True)
