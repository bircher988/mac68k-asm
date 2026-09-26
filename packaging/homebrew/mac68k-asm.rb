# Homebrew formula for mac68k-asm. The published copy lives in github.com/bircher988/homebrew-tap
# (Formula/mac68k-asm.rb); update url and sha256 there for each release.
class Mac68kAsm < Formula
  desc "68k assembler, linker and resource compiler for the classic Macintosh"
  homepage "https://github.com/bircher988/mac68k-asm"
  url "https://github.com/bircher988/mac68k-asm/archive/refs/tags/v1.1.tar.gz"
  sha256 "2f4392b1c3b63b2ed99f9b457fa7491785bedf64833b15838be8093d7290c21c"
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
