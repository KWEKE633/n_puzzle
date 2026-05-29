{
  description = "Zig fixed version development environment";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, utils }:
    utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };

        target-zig-version = "0.16.0";
      in
      {
        devShells.default = pkgs.mkShell {
          buildInputs = [
            pkgs.zig
          ];
        };
      });
}
