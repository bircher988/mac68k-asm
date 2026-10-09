# Homebrew formula for mac68k-asm. The published copy lives in github.com/bircher988/homebrew-tap
# (Formula/mac68k-asm.rb); update url and sha256 there for each release.
class Mac68kAsm < Formula
  desc "68k assembler, linker and resource compiler for the classic Macintosh"
  homepage "https://github.com/bircher988/mac68k-asm"
  url "https://github.com/bircher988/mac68k-asm/archive/refs/tags/v1.2.2.tar.gz"
  sha256 "8d1ac3d0b2ee5b994a0f372dd49661e8fdae1f210edd86a47ff9e3f0ff9ecc45"
  license "MIT"
  head "https://github.com/bircher988/mac68k-asm.git", branch: "main"

  def install
    system "make", "PREFIX=#{prefix}"
    system "make", "install", "PREFIX=#{prefix}"
  end

  test do
    system "#{bin}/mac68k-asm", "version"
    cp_r "#{pkgshare}/example/.", testpath
    system "#{bin}/mac68k-asm", "build", "Hello.Job", "-o", "out"
    assert_predicate testpath/"out/Hello.bin", :exist?
  end
end
