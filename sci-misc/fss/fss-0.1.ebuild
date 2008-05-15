# Copyright 1999-2008 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

# By Kamil Dudka <xdudka00@gmail.com>
# Brno University of Technology
# Faculty of Information Technology

inherit eutils

DESCRIPTION="Fast SAT Solver"

HOMEPAGE="http://dudka.cz/fss"
SRC_URI="http://dudka.cz/fss/html/fss-0.1.tar.bz2"
RESTRICT="mirror"
LICENSE="GPL"
SLOT="0"
KEYWORDS="~amd64 ~x86"
IUSE=""
DEPEND=" \
	dev-util/cmake \
	sci-libs/galib"
RDEPEND=""

src_compile() {
	cd src || die "malformed tarball"
	cmake . || die "cmake failed"
	emake || die "build failed"
}

src_install() {
	dodoc proj_doc.pdf
	dodir /usr/share/${PN}
	cp *.txt ${D}/usr/share/${PN}/
	dobin src/fss src/fss-satgen
}

