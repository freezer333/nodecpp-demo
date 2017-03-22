{
  "targets": [
    {
        "target_name": "typed_example",
        "sources": [ "typed_example.cpp" ], 
        "include_dirs" : ["<!(node -e \"require('nan')\")"]
    }
  ]
}