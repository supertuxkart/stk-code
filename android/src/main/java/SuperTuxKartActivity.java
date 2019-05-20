package org.supertuxkart.stk_dbg;

import android.app.NativeActivity;
import android.content.Context;
import android.graphics.Rect;
import android.os.Bundle;
import android.view.inputmethod.InputMethodManager;
import android.view.KeyEvent;
import android.view.ViewTreeObserver.OnGlobalLayoutListener;
import android.view.View;

public class SuperTuxKartActivity extends NativeActivity
{
    private native void saveFromJavaChars(String chars);
    private native void saveKeyboardHeight(int height);

    @Override
    public void onCreate(Bundle instance)
    {
        super.onCreate(instance);
        System.loadLibrary("main");
        final View root = getWindow().getDecorView().findViewById(
            android.R.id.content);
        root.getViewTreeObserver().addOnGlobalLayoutListener(new
            OnGlobalLayoutListener()
            {
                @Override
                public void onGlobalLayout()
                {
                    Rect r = new Rect();
                    root.getWindowVisibleDisplayFrame(r);
                    int screen_height = root.getRootView().getHeight();
                    int keyboard_height = screen_height - (r.bottom);
                    saveKeyboardHeight(keyboard_height);
                }
            });
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

    public void showKeyboard()
    {
        InputMethodManager imm = (InputMethodManager)
            getSystemService(Context.INPUT_METHOD_SERVICE);
        imm.showSoftInput(getWindow().getDecorView(),
            InputMethodManager.SHOW_FORCED);
    }

    public void hideKeyboard()
    {
        InputMethodManager imm = (InputMethodManager)
            getSystemService(Context.INPUT_METHOD_SERVICE);
        imm.hideSoftInputFromWindow(
            getWindow().getDecorView().getWindowToken(), 0);
    }
}
