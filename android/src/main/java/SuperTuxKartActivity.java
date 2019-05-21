package org.supertuxkart.stk_dbg;

import android.app.NativeActivity;
import android.content.Context;
import android.graphics.Rect;
import android.os.Build;
import android.os.Bundle;
import android.view.inputmethod.InputMethodManager;
import android.view.KeyEvent;
import android.view.ViewTreeObserver.OnGlobalLayoutListener;
import android.view.View;

public class SuperTuxKartActivity extends NativeActivity
{
    private native void saveFromJavaChars(String chars);
    private native void saveKeyboardHeight(int height);

    private void hideNavBar(View decor_view)
    {
        if (Build.VERSION.SDK_INT < 19)
            return;
        decor_view.setSystemUiVisibility(
            View.SYSTEM_UI_FLAG_FULLSCREEN |
            View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
            View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY |
            View.SYSTEM_UI_FLAG_LAYOUT_STABLE |
            View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
            View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION);
    }

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

        final View decor_view = getWindow().getDecorView();
        decor_view.setOnSystemUiVisibilityChangeListener(
            new View.OnSystemUiVisibilityChangeListener()
            {
                @Override
                public void onSystemUiVisibilityChange(int visibility)
                {
                    if ((visibility & View.SYSTEM_UI_FLAG_FULLSCREEN) == 0)
                        hideNavBar(decor_view);
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

    @Override
    public void onWindowFocusChanged(boolean has_focus)
    {
        super.onWindowFocusChanged(has_focus);
        if (has_focus)
            hideNavBar(getWindow().getDecorView());
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
