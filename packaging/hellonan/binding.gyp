{
  "targets": [
    {
      "target_name": "hello_nan_addon",
      "sources": [ "hello_nan.cpp" ], 
      "include_dirs" : [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}