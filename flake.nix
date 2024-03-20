{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-23.11";
  };

  outputs = { self, nixpkgs }:
  let
    system = "x86_64-linux";
    pkgs = nixpkgs.legacyPackages.${system};
  in {
    devShells.${system}.default = pkgs.gcc13Stdenv.mkDerivation {
      name = "pareas-dev";
      nativeBuildInputs = [
        pkgs.meson
        pkgs.futhark
        pkgs.ninja
      ];
    };
  };
}
