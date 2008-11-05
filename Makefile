# Copyright (C) 2008 Kamil Dudka <xdudka00@stud.fit.vutbr.cz>
#
# This file is part of fss (Fast SAT Solver).
# 
# fss is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
# 
# fss is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with fss.  If not, see <http://www.gnu.org/licenses/>.

WGET?=wget
TAR?=tar
ZIP?=zip

FSS_GALIB_BUNDLE=fss-galib
FSS_GALIB_BUNDLE_ZIP=$(FSS_GALIB_BUNDLE).zip
FSS_GALIB_BUNDLE_CONTENT=COPYING data doc $(GALIB_DIR) Makefile README src

GALIB_DIR=galib247
GALIB_TGZ=$(GALIB_DIR).tgz
GALIB_URL="http://lancet.mit.edu/ga/dist/$(GALIB_TGZ)"
GALIB_LIB=$(GALIB_DIR)/ga/libga.a

.PHONY: all build_dir clean distclean galib $(FSS_GALIB_BUNDLE_ZIP)

all: build_dir
	$(MAKE) all -C build

build_dir: $(GALIB_LIB)
	test -d build || mkdir build
	cd build && cmake -D GALIB_DIR=../$(GALIB_DIR) ../src

$(GALIB_LIB): galib
galib: $(GALIB_DIR)
	$(MAKE) lib -C $(GALIB_DIR)

$(GALIB_DIR):
	test -f $(GALIB_TGZ) || $(WGET) $(GALIB_URL)
	$(TAR) xzvf $(GALIB_TGZ)

clean:
	rm -rfv build
	$(MAKE) clean -C doc

distclean: clean
	rm -rfv $(GALIB_DIR) $(GALIB_TGZ)
	rm -rfv $(FSS_GALIB_BUNDLE) $(FSS_GALIB_BUNDLE_ZIP)

$(FSS_GALIB_BUNDLE_ZIP): distclean
	$(MAKE) $(GALIB_DIR)
	mkdir $(FSS_GALIB_BUNDLE)
	cp -Rv $(FSS_GALIB_BUNDLE_CONTENT) $(FSS_GALIB_BUNDLE)
	$(ZIP) -r $@ $(FSS_GALIB_BUNDLE)
