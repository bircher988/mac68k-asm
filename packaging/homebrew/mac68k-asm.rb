# Homebrew formula for mac68k-asm. Lives in a tap (e.g. <user>/homebrew-tap):
#   brew tap <user>/tap && brew install mac68k-asm
# Update url/sha256 for each release: shasum -a 256 mac68k-asm-<version>.tar.gz
class Mac68kAsm < Formula
  desc "68k assembler toolchain for the classic Macintosh (assembler, linker, resource compiler)"
  homepage "https://github.com/bircher988/mac68k-asm"
  url "https://github.com/bircher988/mac68k-asm/archive/refs/tags/v1.0.tar.gz"
  sha256 "0000000000000000000000000000000000000000000000000000000000000000"
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
