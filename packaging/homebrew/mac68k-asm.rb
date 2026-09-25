# Homebrew formula for mac68k-asm. The published copy lives in github.com/bircher988/homebrew-tap
# (Formula/mac68k-asm.rb); update url and sha256 there for each release.
class Mac68kAsm < Formula
  desc "68k assembler, linker and resource compiler for the classic Macintosh"
  homepage "https://github.com/bircher988/mac68k-asm"
  url "https://github.com/bircher988/mac68k-asm/archive/refs/tags/v1.0.tar.gz"
  sha256 "8e369af996009e212be4cd53c698fe40adb9e817ea719b65e37b88eb08aeaf5f"
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
