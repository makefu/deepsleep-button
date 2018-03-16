{ pkgs ? import <nixpkgs> {} }:
let
in pkgs.stdenv.mkDerivation rec {
  name = "minikrebs-env";
  version = "1.1";
  buildInputs = with pkgs; [
    wget
    git
    gawk
    bash
    platformio
    mosquitto
    esptool # nixpkgs
  ];
    shellHook =''
      HISTFILE="${toString ./.}/.histfile"
    '' ;
}
