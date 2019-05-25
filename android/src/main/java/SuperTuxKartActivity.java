package org.supertuxkart.stk_dbg;

import org.supertuxkart.stk_dbg.STKEditText;

import android.app.NativeActivity;
import android.content.Context;
import android.graphics.Rect;
import android.os.Build;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.inputmethod.InputMethodManager;
import android.view.KeyEvent;
import android.view.ViewTreeObserver.OnGlobalLayoutListener;
import android.view.View;
import android.widget.FrameLayout;

public class SuperTuxKartActivity extends NativeActivity
{
    private STKEditText m_stk_edittext;

    // ------------------------------------------------------------------------
    private native void saveKeyboardHeight(int height);
    // ------------------------------------------------------------------------
    private void hideKeyboardNative()
    {
        if (m_stk_edittext == null)
            return;

        m_stk_edittext.beforeHideKeyboard();

        InputMethodManager imm = (InputMethodManager)
            getSystemService(Context.INPUT_METHOD_SERVICE);
        imm.hideSoftInputFromWindow(m_stk_edittext.getWindowToken(), 0);
    }
    // ------------------------------------------------------------------------
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
    // ------------------------------------------------------------------------
    @Override
    public void onCreate(Bundle instance)
    {
        super.onCreate(instance);
        System.loadLibrary("main");
        m_stk_edittext = null;

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
    // ------------------------------------------------------------------------
    @Override
    public void onPause()
    {
        super.onPause();
        hideKeyboardNative();
    }
    // ------------------------------------------------------------------------
    @Override
    public void onWindowFocusChanged(boolean has_focus)
    {
        super.onWindowFocusChanged(has_focus);
        if (has_focus)
            hideNavBar(getWindow().getDecorView());
    }
    // ------------------------------------------------------------------------
    @Override
    public boolean dispatchKeyEvent(KeyEvent event)
    {
        // Called when user change cursor / select all text in native android
        // keyboard
        boolean ret = super.dispatchKeyEvent(event);
        if (m_stk_edittext != null)
            m_stk_edittext.updateSTKEditBox();
        return ret;
    }
    // ------------------------------------------------------------------------
    public void showKeyboard()
    {
        final Context context = this;
        // Need to run in ui thread as it access the view m_stk_edittext
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                InputMethodManager imm = (InputMethodManager)
                    getSystemService(Context.INPUT_METHOD_SERVICE);

                FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(
                    FrameLayout.LayoutParams.WRAP_CONTENT,
                    FrameLayout.LayoutParams.WRAP_CONTENT);
                if (m_stk_edittext == null)
                {
                    m_stk_edittext = new STKEditText(context);
                    // For some copy-and-paste text are not done by commitText
                    // in STKInputConnection, so we need an extra watcher
                    m_stk_edittext.addTextChangedListener(new TextWatcher()
                        {
                            @Override
                            public void onTextChanged(CharSequence s,
                                                      int start, int before,
                                                      int count) {}
                            @Override
                            public void beforeTextChanged(CharSequence s,
                                                          int start, int count,
                                                          int after) {}
                            @Override
                            public void afterTextChanged(Editable edit)
                            {
                                if (m_stk_edittext != null)
                                    m_stk_edittext.updateSTKEditBox();
                            }
                        });
                    addContentView(m_stk_edittext, params);
                }
                else
                    m_stk_edittext.setLayoutParams(params);

                m_stk_edittext.resetWhenFocus();
                m_stk_edittext.setVisibility(View.VISIBLE);
                m_stk_edittext.requestFocus();

                imm.showSoftInput(m_stk_edittext,
                    InputMethodManager.SHOW_FORCED);
            }
        });
    }
    // ------------------------------------------------------------------------
    /* Called by STK in JNI. */
    public void hideKeyboard()
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                hideKeyboardNative();
            }
        });
    }
}
