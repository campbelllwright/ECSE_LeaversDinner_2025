{
  description = "Development environment for Raspberry Pi Pico";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs";

  outputs =
    {
      self,
      nixpkgs,
    }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
      local-pico-sdk = pkgs.pico-sdk.overrideAttrs (_: {
        withSubmodules = true;
      });
    in
    {
      devShells.${system}.default = pkgs.mkShell rec {
        buildInputs = with pkgs; [
          # Required for C/C++ RPi Pico development
          # Raspberry Pi Pico SDKs
          local-pico-sdk
          picotool

          # Compilers/interpreters
          gcc
          gcc-arm-embedded

          # Needed to make
          ninja
          gnumake
          cmake
          extra-cmake-modules

          # Libraries required for runtime
          libusb1
          libftdi1
          hidapi
          zstd
        ];

        shellHook = ''
          export PICO_SDK_PATH="${local-pico-sdk}/lib/pico-sdk"
          export LD_LIBRARY_PATH="${pkgs.stdenv.cc.cc.lib}/lib:${pkgs.lib.makeLibraryPath buildInputs}:$LD_LIBRARY_PATH"
        '';
      };
    };
}
