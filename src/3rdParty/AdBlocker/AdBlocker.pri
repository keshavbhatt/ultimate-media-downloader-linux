INCLUDEPATH += $$PWD/ad-block
INCLUDEPATH += $$PWD/bloom-filter-cpp
INCLUDEPATH += $$PWD/hashset-cpp

QT += webengine

SOURCES += $$PWD/ad-block/ad_block_client.cc \
           $$PWD/ad-block/context_domain.cc \
           $$PWD/ad-block/cosmetic_filter.cc \
           $$PWD/ad-block/filter.cc \
           $$PWD/ad-block/no_fingerprint_domain.cc \
           $$PWD/ad-block/protocol.cc \
           $$PWD/bloom-filter-cpp/BloomFilter.cpp \
           $$PWD/easylistmanager.cpp \
           $$PWD/hashset-cpp/hashFn.cc \
           $$PWD/hashset-cpp/hash_set.cc \
           $$PWD/requestinterceptor.cpp

HEADERS += $$PWD/ad-block/ad_block_client.h \
           $$PWD/ad-block/bad_fingerprint.h \
           $$PWD/ad-block/bad_fingerprints.h \
           $$PWD/ad-block/context_domain.h \
           $$PWD/ad-block/cosmetic_filter.h \
           $$PWD/ad-block/filter.h \
           $$PWD/ad-block/no_fingerprint_domain.h \
           $$PWD/ad-block/protocol.h \
           $$PWD/ad-block/base.h \
           $$PWD/bloom-filter-cpp/base.h \
           $$PWD/bloom-filter-cpp/BloomFilter.h \
           $$PWD/easylistmanager.h \
           $$PWD/hashset-cpp/hash_item.h \
           $$PWD/hashset-cpp/base.h \
           $$PWD/hashset-cpp/hashFn.h \
           $$PWD/hashset-cpp/hash_set.h \
           $$PWD/requestinterceptor.h
