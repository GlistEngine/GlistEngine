package dev.glist.android;

import android.os.Bundle;
import dev.glist.glistapp.BuildConfig;

public class GlistAppActivity extends BaseGlistAppActivity {

    static {
        LIBRARY_NAME = BuildConfig.LIBRARY_NAME;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // Your settings can go here.
        //GlistNative.setFullscreen(true); // Uncomment this line to hide status bar.
    }
}