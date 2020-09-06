# Maintainer: Giovanni Scafora <giovanni@archlinux.org>
# Contributor: Tom Newsom <Jeepster@gmx.co.uk>

# ALARM: Kevin Mihelich <kevin@archlinuxarm.org>
#  - configure with --enable-mpers=check to fix AArch64 build

pkgname=rpi-fan
pkgver=1.0
pkgrel=1
pkgdesc='A program to manage a fan run connected to GPIO pins.'
arch=('armv7h' 'aarch64')
url='https://github.com/njordan64/rpi-fan'
license=(GPL2)
depends=('raspberrypi-userland-aarch64-git')
source=('rpi_fan.c'
        'Makefile'
        'COPYING'
        'rpi-fan.service')
sha256sums=('4d88ff94a388fae46704c4c33aa04ea7c0d4d9ae0461d1d51fa1f3aa34a14e01'
            '6421dd1bf1e65bb2ad09e1004a0d1a3a3d6c927957b52ccff6753659cd470682'
            '9da63520e9293d82c4b1613be5c84058cadb82b02f5972179bad13731d589910'
            '8e92d38f078e31ac495edba6fc42eea079246c43e8b2d449031abb09f0b201fd')

build() {
  make
}

check() {
  # tests do not work in chroot environment. TODO: fixit.
  # make -C $pkgname-$pkgver check
  true
}

package() {
  install -Dm755 rpi-fan "$pkgdir/usr/sbin/rpi-fan"
  install -Dm644 rpi-fan.service "$pkgdir/lib/systemd/system/rpi-fan.service"
  install -Dm644 COPYING "$pkgdir/usr/share/licenses/$pkgname/LICENSE"
}
