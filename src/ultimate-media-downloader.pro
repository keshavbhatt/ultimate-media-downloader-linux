#-------------------------------------------------
#
# Project created by QtCreator 2020-04-29T11:53:19
#
#-------------------------------------------------

QT       += core gui network

include(3rdParty/QRateApp/RateApp.pri)

include(3rdParty/Account/Account.pri)

include(3rdParty/GumboQuery/GumboQuery.pri)

include(3rdParty/MoreApps/MoreApps.pri)

include(3rdParty/Tracking/Tracking.pri)

include(3rdParty/WebBrowser/WebBrowser.pri)

include(3rdParty/AdBlocker/AdBlocker.pri)

include(3rdParty/TabWidget/TabWidget.pri)

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

TARGET = ultimate-media-downloader
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++14

# Define git info
GIT_HASH="\\\"$$system(git -C \""$$_PRO_FILE_PWD_"\" rev-parse --short HEAD)\\\""
GIT_BRANCH="\\\"$$system(git -C \""$$_PRO_FILE_PWD_"\" rev-parse --abbrev-ref HEAD)\\\""
BUILD_TIMESTAMP="\\\"$$system(date -u +\""%Y-%m-%dT%H:%M:%SUTC\"")\\\""
DEFINES += GIT_HASH=$$GIT_HASH GIT_BRANCH=$$GIT_BRANCH BUILD_TIMESTAMP=$$BUILD_TIMESTAMP

VERSION = 5.3
DEFINES += VERSIONSTR=\\\"$${VERSION}\\\"

APPLICATION_NAME= UMD4
DEFINES += APPLICATION_NAME=\\\"$${APPLICATION_NAME}\\\"

APPLICATION_FULLNAME= Ultimate-Media-Downloader
DEFINES += APPLICATION_FULLNAME=\\\"$${APPLICATION_FULLNAME}\\\"

LIBS += -lgumbo

SOURCES += \
    about/about.cpp \
    accountmanager.cpp \
    basicdownloader.cpp \
    batchdownloader.cpp \
    configure.cpp \
    customformatselector.cpp \
    downloadmanager.cpp \
    engines/generic.cpp \
    engineupdatecheck.cpp \
    logger.cpp \
    networkAccess.cpp \
    main.cpp \
    engines.cpp \
    services/invidiousinstances.cpp \
    services/invidioustrendingparser.cpp \
    services/playerservice.cpp \
    services/xp/xpcategories.cpp \
    services/xp/xpsearchparser.cpp \
    services/xp/xpsearchservice.cpp \
    services/xp/xptriggerwords.cpp \
    services/ytdlgetformat.cpp \
    services/ytsearchservice.cpp \
    supportedsites.cpp \
    translator.cpp \
    mainwindow.cpp \
    playlistdownloader.cpp \
    library.cpp \
    tableWidget.cpp \
    settings.cpp \
    tabmanager.cpp \
    trendingwidget.cpp \
    utility.cpp \
    logwindow.cpp \
    engines/youtube-dl.cpp \
    widgets/RangeSlider.cpp \
    widgets/ccheckboxitemdelegate.cpp \
    widgets/controlbutton.cpp \
    widgets/cverticallabel.cpp \
    widgets/elidedlabel.cpp \
    widgets/remotepixmaplabel2.cpp \
    widgets/spoiler.cpp \
    widgets/waitingspinnerwidget.cpp \
    xpsearch.cpp \
    xpvideo.cpp \
    ytplaylist.cpp \
    ytsearch.cpp \
    ytvideo.cpp

HEADERS += \
    about/about.h \
    accountmanager.h \
    basicdownloader.h \
    batchdownloader.h \
    configure.h \
    customformatselector.h \
    downloadmanager.h \
    engines.h \
    engines/generic.h \
    engines/youtube-dl.h \
    engineupdatecheck.h \
    library.h \
    logger.h \
    logwindow.h \
    mainwindow.h \
    networkAccess.h \
    playlistdownloader.h \
    services/invidiousinstances.h \
    services/invidioustrendingparser.h \
    services/playerservice.h \
    services/xp/xpcategories.h \
    services/xp/xpsearchparser.h \
    services/xp/xpsearchservice.h \
    services/xp/xptriggerwords.h \
    services/ytdlgetformat.h \
    services/ytsearchservice.h \
    settings.h \
    supportedsites.h \
    tableWidget.h \
    tabmanager.h \
    translator.h \
    trendingwidget.h \
    utility.h \
    widgets/RangeSlider.h \
    widgets/ccheckboxitemdelegate.h \
    widgets/controlbutton.h \
    widgets/cverticallabel.h \
    widgets/elidedlabel.h \
    widgets/remotepixmaplabel2.h \
    widgets/spoiler.h \
    widgets/waitingspinnerwidget.h \
    xpsearch.h \
    xpvideo.h \
    ytplaylist.h \
    ytsearch.h \
    ytvideo.h

FORMS += \
    about/about.ui \
    customformatselector.ui \
    mainwindow.ui \
    logwindow.ui \
    trendingwidget.ui \
    xpvideo.ui \
    ytplaylist.ui \
    ytvideo.ui

TRANSLATIONS = \
    ../translations/en_US.ts \
    ../translations/pl_PL.ts \
    ../translations/zh_CN.ts \
    ../translations/es.ts \
    ../translations/tr_TR.ts \
    ../translations/ru_RU.ts \
    ../translations/ja_JP.ts

RESOURCES += \
    icons.qrc \
    others.qrc

DISTFILES += \
    icons/flags/ad.png \
    icons/flags/ae.png \
    icons/flags/af.png \
    icons/flags/ag.png \
    icons/flags/ai.png \
    icons/flags/al.png \
    icons/flags/am.png \
    icons/flags/ao.png \
    icons/flags/aq.png \
    icons/flags/ar.png \
    icons/flags/as.png \
    icons/flags/at.png \
    icons/flags/au.png \
    icons/flags/aw.png \
    icons/flags/ax.png \
    icons/flags/az.png \
    icons/flags/ba.png \
    icons/flags/bb.png \
    icons/flags/bd.png \
    icons/flags/be.png \
    icons/flags/bf.png \
    icons/flags/bg.png \
    icons/flags/bh.png \
    icons/flags/bi.png \
    icons/flags/bj.png \
    icons/flags/bl.png \
    icons/flags/bm.png \
    icons/flags/bn.png \
    icons/flags/bo.png \
    icons/flags/bq.png \
    icons/flags/br.png \
    icons/flags/bs.png \
    icons/flags/bt.png \
    icons/flags/bv.png \
    icons/flags/bw.png \
    icons/flags/by.png \
    icons/flags/bz.png \
    icons/flags/ca.png \
    icons/flags/cc.png \
    icons/flags/cd.png \
    icons/flags/cf.png \
    icons/flags/cg.png \
    icons/flags/ch.png \
    icons/flags/ci.png \
    icons/flags/ck.png \
    icons/flags/cl.png \
    icons/flags/cm.png \
    icons/flags/cn.png \
    icons/flags/co.png \
    icons/flags/cr.png \
    icons/flags/cu.png \
    icons/flags/cv.png \
    icons/flags/cw.png \
    icons/flags/cx.png \
    icons/flags/cy.png \
    icons/flags/cz.png \
    icons/flags/de.png \
    icons/flags/dj.png \
    icons/flags/dk.png \
    icons/flags/dm.png \
    icons/flags/do.png \
    icons/flags/dz.png \
    icons/flags/ec.png \
    icons/flags/ee.png \
    icons/flags/eg.png \
    icons/flags/eh.png \
    icons/flags/er.png \
    icons/flags/es.png \
    icons/flags/et.png \
    icons/flags/fi.png \
    icons/flags/fj.png \
    icons/flags/fk.png \
    icons/flags/fm.png \
    icons/flags/fo.png \
    icons/flags/fr.png \
    icons/flags/ga.png \
    icons/flags/gb.png \
    icons/flags/gd.png \
    icons/flags/ge.png \
    icons/flags/gf.png \
    icons/flags/gg.png \
    icons/flags/gh.png \
    icons/flags/gi.png \
    icons/flags/gl.png \
    icons/flags/gm.png \
    icons/flags/gn.png \
    icons/flags/gp.png \
    icons/flags/gq.png \
    icons/flags/gr.png \
    icons/flags/gs.png \
    icons/flags/gt.png \
    icons/flags/gu.png \
    icons/flags/gw.png \
    icons/flags/gy.png \
    icons/flags/hk.png \
    icons/flags/hm.png \
    icons/flags/hn.png \
    icons/flags/hr.png \
    icons/flags/ht.png \
    icons/flags/hu.png \
    icons/flags/id.png \
    icons/flags/ie.png \
    icons/flags/il.png \
    icons/flags/im.png \
    icons/flags/in.png \
    icons/flags/io.png \
    icons/flags/iq.png \
    icons/flags/ir.png \
    icons/flags/is.png \
    icons/flags/it.png \
    icons/flags/je.png \
    icons/flags/jm.png \
    icons/flags/jo.png \
    icons/flags/jp.png \
    icons/flags/ke.png \
    icons/flags/kg.png \
    icons/flags/kh.png \
    icons/flags/ki.png \
    icons/flags/km.png \
    icons/flags/kn.png \
    icons/flags/kp.png \
    icons/flags/kr.png \
    icons/flags/kw.png \
    icons/flags/ky.png \
    icons/flags/kz.png \
    icons/flags/la.png \
    icons/flags/lb.png \
    icons/flags/lc.png \
    icons/flags/li.png \
    icons/flags/lk.png \
    icons/flags/lr.png \
    icons/flags/ls.png \
    icons/flags/lt.png \
    icons/flags/lu.png \
    icons/flags/lv.png \
    icons/flags/ly.png \
    icons/flags/ma.png \
    icons/flags/mc.png \
    icons/flags/md.png \
    icons/flags/me.png \
    icons/flags/mf.png \
    icons/flags/mg.png \
    icons/flags/mh.png \
    icons/flags/mk.png \
    icons/flags/ml.png \
    icons/flags/mm.png \
    icons/flags/mn.png \
    icons/flags/mo.png \
    icons/flags/mp.png \
    icons/flags/mq.png \
    icons/flags/mr.png \
    icons/flags/ms.png \
    icons/flags/mt.png \
    icons/flags/mu.png \
    icons/flags/mv.png \
    icons/flags/mw.png \
    icons/flags/mx.png \
    icons/flags/my.png \
    icons/flags/mz.png \
    icons/flags/na.png \
    icons/flags/nc.png \
    icons/flags/ne.png \
    icons/flags/nf.png \
    icons/flags/ng.png \
    icons/flags/ni.png \
    icons/flags/nl.png \
    icons/flags/no.png \
    icons/flags/np.png \
    icons/flags/nr.png \
    icons/flags/nu.png \
    icons/flags/nz.png \
    icons/flags/om.png \
    icons/flags/pa.png \
    icons/flags/pe.png \
    icons/flags/pf.png \
    icons/flags/pg.png \
    icons/flags/ph.png \
    icons/flags/pk.png \
    icons/flags/pl.png \
    icons/flags/pm.png \
    icons/flags/pn.png \
    icons/flags/pr.png \
    icons/flags/ps.png \
    icons/flags/pt.png \
    icons/flags/pw.png \
    icons/flags/py.png \
    icons/flags/qa.png \
    icons/flags/re.png \
    icons/flags/ro.png \
    icons/flags/rs.png \
    icons/flags/ru.png \
    icons/flags/rw.png \
    icons/flags/sa.png \
    icons/flags/sb.png \
    icons/flags/sc.png \
    icons/flags/sd.png \
    icons/flags/se.png \
    icons/flags/sg.png \
    icons/flags/sh.png \
    icons/flags/si.png \
    icons/flags/sj.png \
    icons/flags/sk.png \
    icons/flags/sl.png \
    icons/flags/sm.png \
    icons/flags/sn.png \
    icons/flags/so.png \
    icons/flags/sr.png \
    icons/flags/ss.png \
    icons/flags/st.png \
    icons/flags/sv.png \
    icons/flags/sx.png \
    icons/flags/sy.png \
    icons/flags/sz.png \
    icons/flags/tc.png \
    icons/flags/td.png \
    icons/flags/tf.png \
    icons/flags/tg.png \
    icons/flags/th.png \
    icons/flags/tj.png \
    icons/flags/tk.png \
    icons/flags/tl.png \
    icons/flags/tm.png \
    icons/flags/tn.png \
    icons/flags/to.png \
    icons/flags/tr.png \
    icons/flags/tt.png \
    icons/flags/tv.png \
    icons/flags/tw.png \
    icons/flags/tz.png \
    icons/flags/ua.png \
    icons/flags/ug.png \
    icons/flags/um.png \
    icons/flags/us.png \
    icons/flags/uy.png \
    icons/flags/uz.png \
    icons/flags/va.png \
    icons/flags/vc.png \
    icons/flags/ve.png \
    icons/flags/vg.png \
    icons/flags/vi.png \
    icons/flags/vn.png \
    icons/flags/vu.png \
    icons/flags/wf.png \
    icons/flags/ws.png \
    icons/flags/xx.png \
    icons/flags/ye.png \
    icons/flags/yt.png \
    icons/flags/za.png \
    icons/flags/zm.png \
    icons/flags/zw.png \
    others/regions
