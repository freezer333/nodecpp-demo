{
  "targets": [
    {
      "target_name": "native_rt",
      "sources": [ "native-rt.cc" ],
      "conditions":[
      	["OS=='linux'", {
      	  "sources": [ "native-rt_linux.cc" ]
      	  }],
      	["OS=='mac'", {
      	  "sources": [ "native-rt_mac.cc" ]
      	}],
        ["OS=='win'", {
      	  "sources": [ "native-rt_win.cc" ]
      	}]
      ], 
      "include_dirs" : [
        "<!(node -e \"require('nan')\")"
      ]
    }, 
    # IMPORTANT:  This action is critical for a node-pre-gyp module, 
    # it makes sure the binary is placed into the correct directory
    # so requiring apps can find it (see index.js)
    {
      "target_name": "action_after_build",
      "type": "none",
      "dependencies": [ "<(module_name)" ],
      "copies": [
        {
          "files": [ "<(PRODUCT_DIR)/<(module_name).node" ],
          "destination": "<(module_path)"
        }
      ]
    }

  ]
}