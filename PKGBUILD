# Maintainer: valad47 <valad.racz@gmail.com>
pkgname=vlm
pkgver=0.3
pkgrel=2
pkgdesc="Luau package manager and runtime"
arch=('x86_64')
url="https://github.com/valad47/vlm"
license=('unknown')
depends=(gcc-libs glibc tar gzip)
makedepends=(cmake git clang)
source=("git+https://github.com/valad47/vlm.git")
sha256sums=('SKIP')

prepare() {
	cd "$pkgname"
}

build() {
	cd "$pkgname"
    cmake -B build \
    	-DCMAKE_BUILD_TYPE=Release \
    	-DCMAKE_C_COMPILER=clang
    
    cmake --build build -j4 -t vlm vlmruntime

    mkdir -p include/vlm

    cp build/_deps/luau-src/VM/include/*.h \
       build/_deps/luau-src/Compiler/include/luacode.h \
       "$srcdir/$pkgname/include/vlm/"
}

check() {
	cd "$pkgname"
}

package() {
	cd "$pkgname"
	install -Dm0755 -t "$pkgdir/usr/bin/" build/vlm
    install -Dm0755 -t "$pkgdir/usr/lib/" build/libvlmruntime.so 
    install -Dm0644 -t "$pkgdir/usr/include/vlm/" include/vlm/*
}
