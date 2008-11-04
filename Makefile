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

.PHONY: all build_dir clean

all: build_dir
	$(MAKE) all -C build

build_dir:
	test -d build || mkdir build
	cd build && cmake ../src

clean:
	rm -rfv build
