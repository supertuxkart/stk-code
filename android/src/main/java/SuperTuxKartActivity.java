package org.supertuxkart.stk_dbg;

import android.app.NativeActivity;
import android.os.Bundle;
import android.view.KeyEvent;

public class SuperTuxKartActivity extends NativeActivity
{
    private native void saveFromJavaChars(String chars);

    @Override
    public void onCreate(Bundle instance)
    {
        super.onCreate(instance);
        System.loadLibrary("main");
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event)
    {
        // ACTION_MULTIPLE deprecated in API level Q, it says if the key code
        // is KEYCODE_UNKNOWN, then this is a sequence of characters as
        // returned by getCharacters()
        if (event.getKeyCode() == KeyEvent.KEYCODE_UNKNOWN &&
            event.getAction() == KeyEvent.ACTION_MULTIPLE)
        {
            String chars = event.getCharacters();
            if (chars != null)
            {
                saveFromJavaChars(chars);
                return true;
            }
        }
        return super.dispatchKeyEvent(event);
    }
}
