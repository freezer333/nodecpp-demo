{
  "targets": [
    {
      "target_name": "my_addon",
      "sources": [ "addon_source.cc" ], 
      "include_dirs" : [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}