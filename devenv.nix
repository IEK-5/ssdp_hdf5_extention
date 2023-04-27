{ pkgs, ... }:

{
  # https://devenv.sh/basics/
  env.GREET = "devenv";

  # https://devenv.sh/packages/
  packages = [ 
    pkgs.git
    pkgs.llvm
    pkgs.hdf5
    pkgs.zlib
    pkgs.hdfview
    pkgs.clang-tools
    pkgs.valgrind
    pkgs.criterion
    pkgs.splint
  ];

  # https://devenv.sh/scripts/
  scripts.hello.exec = "echo hello from $GREET";
  # ''is a multilinestring''
  # it runs things when entering the shell
  enterShell = ''
    hello
    git --version
  '';

  # https://devenv.sh/languages/
  # languages.nix.enable = true;
  languages.c.enable = true;

  # https://devenv.sh/pre-commit-hooks/
  # pre-commit.hooks.shellcheck.enable = true;

  # https://devenv.sh/processes/
  # processes.ping.exec = "ping example.com";

  # See full reference at https://devenv.sh/reference/options/
}
