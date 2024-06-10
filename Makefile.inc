# uncomment and edit this line to link with a static openssl
#STATIC_SSL_PATH := /path/to/openssl

# relative path to the ui to use
UI_PATH := src/ui

# path where the data file will be stored
DATA_FILE := "\"~/.cbftp/data\""

UNAME_S := $(shell uname -s)
# test for different os-release values
# https://www.freedesktop.org/software/systemd/man/latest/os-release.html#:~:text=Example%C2%A03.%C2%A0Reading%20os%2Drelease%20in%20sh(1)
ifneq ($(wildcard /etc/os-release),)
OS_RELEASE := /etc/os-release
else ifneq ($(wildcard /usr/lib/os-release),)
OS_RELEASE := /usr/lib/os-release
endif

ifneq ($(STATIC_SSL_PATH),)
STATIC_SSL_INCLUDE := -I$(STATIC_SSL_PATH)/include
STATIC_SSL_LINK := $(STATIC_SSL_PATH)/libssl.a $(STATIC_SSL_PATH)/libcrypto.a
SSL_LINKFLAGS := $(STATIC_SSL_LINK) -ldl
else
SSL_LINKFLAGS := -lssl -lcrypto
endif

ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

HAS_SVN := $(shell svn info $(ROOT_DIR) > /dev/null 2>&1; echo $$?)
REDIST_VERSION := $(shell cat $(ROOT_DIR)/.version 2>/dev/null)

ifeq ($(HAS_SVN), 0)
VERSION := "\"svn:r$(shell svn info $(ROOT_DIR)|grep Revision|awk '{ print $$2 }')\""
SVN_ROOT_DIR := $(shell svn info $(ROOT_DIR)|grep 'Working Copy Root'|awk '{print $$5 }')
else ifneq ($(REDIST_VERSION),)
VERSION := "\"redist:r$(REDIST_VERSION)\""
else
VERSION := "\"unknown\""
endif

DATE := "\"$(shell date)\""

BOXTAG := "\"cbftp\""

CXX ?= g++
ifndef OPTFLAGS
OPTFLAGS = -g -O0
else
$(info Using OPTFLAGS provided by shell)
endif
ifndef CXXFLAGS
CXXFLAGS = $(OPTFLAGS) -std=c++11 -Wall -MMD -MP
ifdef STATIC_SSL_INCLUDE
CXXFLAGS += $(STATIC_SSL_INCLUDE)
endif
else
$(info Using CXXFLAGS provided by shell)
endif
ifndef CFLAGS
CFLAGS = $(OPTFLAGS) -Wall -MMD -MP
else
$(info Using CFLAGS provided by shell)
endif
ifndef LINKFLAGS
  LINKFLAGS=$(SSL_LINKFLAGS) -lpthread
  ifneq ($(wildcard /etc/debian_version),)
    DEBIAN=1
  endif
  ifneq ($(filter $(UNAME_S),FreeBSD),)
    FREEBSD=1
  endif
  ifdef OS_RELEASE
    # Debian-based distributions like to store their ID variable without quotes while Fedora/RHEL will store with
    OS_RELEASE_ID := $(patsubst ID="%",%,$(filter ID=%,$(shell cat $(OS_RELEASE))))
    # ID_LIKE contains spaces, which filter doesnt take too kindly to.
    OS_RELEASE_ID_LIKE := $(subst $\",,$(patsubst ID_LIKE=%,%,$(filter ID_LIKE=%,$(shell cat $(OS_RELEASE)))))
    ifeq ($(OS_RELEASE_ID),rhel)
      REDHAT=1
    else ifeq ($(OS_RELEASE_ID),fedora)
      FEDORA=1
    else ifeq ($(OS_RELEASE_ID_LIKE),rhel)
      REDHAT=1
    else ifeq ($(OS_RELEASE_ID_LIKE),fedora)
      FEDORA=1
    endif
  endif
  ifneq ($(DEBIAN)$(FREEBSD)$(REDHAT)$(FEDORA),)
    LINKFLAGS+=-lncursesw
  else
    LINKFLAGS+=-lncurses
  endif
else
  $(info Using LINKFLAGS provided by shell)
endif

