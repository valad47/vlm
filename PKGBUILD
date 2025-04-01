# Maintainer: valad47 <valad.racz@gmail.com>
pkgname=vlm
pkgver=0.2
pkgrel=1
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
          -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
          -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_INSTALL_PREFIX=/usr \
          -DCMAKE_C_COMPILER=clang
    cmake --build build -j4

    mkdir -p lib/vlm
    mkdir -p include/vlm

    cd build/_deps/luau-build/
    cp libLuau.VM.a \
       libLuau.Compiler.a \
       libLuau.Ast.a \
       "$srcdir/$pkgname/lib/vlm/"

    cd ../luau-src/
    cp VM/include/*.h \
       Compiler/include/luacode.h \
       "$srcdir/$pkgname/include/vlm/"
}

check() {
	cd "$pkgname"
}

package() {
	cd "$pkgname"
	install -Dm0755 -t "$pkgdir/usr/bin/" build/vlm
    install -Dm0644 -t "$pkgdir/usr/lib/vlm/" lib/vlm/*
    install -Dm0644 -t "$pkgdir/usr/include/vlm/" include/vlm/*
}
