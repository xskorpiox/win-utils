{
  "targets": [
    {
      "target_name": "win-utils",
      "sources": [ "src/audio/audio.cpp" ],
      "include_dirs": [
        "<!@(node -e \"require('node-addon-api').include\")",
        "node_modules\\node-addon-api",
        "headers"
      ],
      "dependencies": [
        "<!@(node -e \"require('node-addon-api').gyp\")"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
      "libraries": [
        "-lpropsys",
      ],
    }
  ]
}