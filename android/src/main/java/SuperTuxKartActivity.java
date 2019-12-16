package org.supertuxkart.stk_dbg;

import org.supertuxkart.stk_dbg.STKEditText;

import android.app.NativeActivity;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Rect;
import android.net.Uri;
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
    private void hideKeyboardNative(final boolean clear_text)
    {
        if (m_stk_edittext == null)
            return;

        m_stk_edittext.beforeHideKeyboard(clear_text);

        InputMethodManager imm = (InputMethodManager)
            getSystemService(Context.INPUT_METHOD_SERVICE);
        if (imm == null)
            return;

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
    private void createSTKEditText()
    {
        FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(
            FrameLayout.LayoutParams.WRAP_CONTENT,
            FrameLayout.LayoutParams.WRAP_CONTENT);
        m_stk_edittext = new STKEditText(this);
        // For some copy-and-paste text are not done by commitText in
        // STKInputConnection, so we need an extra watcher
        m_stk_edittext.addTextChangedListener(new TextWatcher()
        {
            @Override
            public void onTextChanged(CharSequence s, int start, int before,
                                      int count) {}
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count,
                                          int after) {}
            @Override
            public void afterTextChanged(Editable edit)
            {
                if (m_stk_edittext != null)
                    m_stk_edittext.updateSTKEditBox();
            }
        });
        addContentView(m_stk_edittext, params);
        // Only focus it and make visible when soft keybord is opened
        m_stk_edittext.setVisibility(View.GONE);
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
        hideKeyboardNative(false/*clear_text*/);
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
    public void showKeyboard(final int type)
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
                if (imm == null)
                    return;

                if (m_stk_edittext == null)
                    createSTKEditText();

                m_stk_edittext.configType(type);
                m_stk_edittext.setVisibility(View.VISIBLE);
                m_stk_edittext.requestFocus();

                imm.showSoftInput(m_stk_edittext,
                    InputMethodManager.SHOW_FORCED);
            }
        });
    }
    // ------------------------------------------------------------------------
    /* Called by STK in JNI. */
    public void hideKeyboard(final boolean clear_text)
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                hideKeyboardNative(clear_text);
            }
        });
    }
    // ------------------------------------------------------------------------
    /* Called by STK in JNI. */
    public void openURL(final String url)
    {
        try
        {
            Uri uri = Uri.parse(url);
            Intent i = new Intent(Intent.ACTION_VIEW, uri);
            if (i.resolveActivity(getPackageManager()) != null)
                startActivity(i);
        }
        catch (ActivityNotFoundException e)
        {
            e.printStackTrace();
        }
    }
    // ------------------------------------------------------------------------
    /* Called by STK in JNI. */
    public void fromSTKEditBox(final int widget_id, final String text,
                               final int selection_start,
                               final int selection_end, final int type)
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                if (m_stk_edittext == null)
                    createSTKEditText();
                m_stk_edittext.configType(type);
                m_stk_edittext.setTextFromSTK(widget_id, text, selection_start,
                    selection_end);
            }
        });
    }
}
