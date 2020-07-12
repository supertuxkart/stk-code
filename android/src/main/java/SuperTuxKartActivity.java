package org.supertuxkart.stk_dbg;

import org.supertuxkart.stk_dbg.STKEditText;
import org.libsdl.app.SDLActivity;

import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.graphics.Rect;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.inputmethod.InputMethodManager;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.ViewGroup.MarginLayoutParams;
import android.view.ViewTreeObserver.OnGlobalLayoutListener;
import android.view.View;
import android.widget.FrameLayout;

import java.util.Set;

import org.minidns.hla.DnssecResolverApi;
import org.minidns.hla.ResolverResult;
import org.minidns.record.SRV;
import org.minidns.record.TXT;

public class SuperTuxKartActivity extends SDLActivity
{
    private STKEditText m_stk_edittext;
    private int m_bottom_y;
    // ------------------------------------------------------------------------
    private native void saveKeyboardHeight(int height);
    // ------------------------------------------------------------------------
    private native static void addDNSSrvRecords(String name, int weight);
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
    private void createSTKEditText()
    {
        FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(
            FrameLayout.LayoutParams.WRAP_CONTENT,
            FrameLayout.LayoutParams.WRAP_CONTENT);
        // We move the dummy edittext out of the android screen because we draw
        // our own manually
        params.setMargins(0, -100000, 1, -100010);
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
        m_bottom_y = 0;
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
                    int moved_height = 0;
                    int margin = screen_height - m_bottom_y;
                    if (keyboard_height > margin)
                        moved_height = -keyboard_height + margin;
                    SDLActivity.moveView(moved_height);
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

    /* STK statically link SDL2. */
    protected String[] getLibraries()
    {
        return new String[]{ "main" };
    }
    // ------------------------------------------------------------------------
    public void showKeyboard(final int type, final int y)
    {
        final Context context = this;
        // Need to run in ui thread as it access the view m_stk_edittext
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                m_bottom_y = y;
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
                m_bottom_y = 0;
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
    // ------------------------------------------------------------------------
    public String[] getDNSTxtRecords(String domain)
    {
        try
        {
            ResolverResult<TXT> txts =
                DnssecResolverApi.INSTANCE.resolve(domain, TXT.class);
            Set<TXT> ans = txts.getAnswers();
            String[] result = new String[ans.size()];
            int i = 0;
            for (TXT t : ans)
                result[i++] = t.getText();
            return result;
        }
        catch (Exception e)
        {
            e.printStackTrace();
            return new String[0];
        }
    }
    // ------------------------------------------------------------------------
    public void getDNSSrvRecords(String domain)
    {
        try
        {
            ResolverResult<SRV> srvs =
                DnssecResolverApi.INSTANCE.resolve(domain, SRV.class);
            Set<SRV> ans = srvs.getAnswers();
            for (SRV s : ans)
                addDNSSrvRecords(s.target.toString() + ":" + s.port, s.weight);
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }
    // ------------------------------------------------------------------------
    public boolean isHardwareKeyboardConnected()
    {
        return getResources().getConfiguration()
            .keyboard == Configuration.KEYBOARD_QWERTY;
    }
    // ------------------------------------------------------------------------
    public int getScreenSize()
    {
        return getResources().getConfiguration().screenLayout &
            Configuration.SCREENLAYOUT_SIZE_MASK;
    }
}
