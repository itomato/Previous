# Previous-Xcode
Xcode files for building Previous on macOS.

Created with `cmake -G Xcode`, modified in Xcode 9 (9A235) to include /Library/Frameworks.

# Use

You'll need Xcode and SDL2.framework.

Run:
    
    git clone https://github.com/itomato/Previous-Xcode
    cd Previous-Xcode
    xcodebuild -project Previous.xcodeproj
    
This should produce Previous.app in src/ and Previous.build/Debug/

    open src/Previous.app
    
The same project and workspace files can be used in the Xcode GUI.
