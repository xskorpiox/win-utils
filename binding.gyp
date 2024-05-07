{
  "targets": [
    {
      "target_name": "win-utils",
      "sources": [ "src/main.cc" ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
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
        "setupapi.lib",
      ],
    }
  ]
}