package org.supertuxkart.stk_dbg;

import org.supertuxkart.stk_dbg.STKEditText;
import org.libsdl.app.SDLActivity;
import org.libsdl.app.SDL;

import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.graphics.Rect;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Process;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.Display;
import android.view.Gravity;
import android.view.inputmethod.InputMethodManager;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.ViewGroup;
import android.view.ViewGroup.MarginLayoutParams;
import android.view.ViewTreeObserver.OnGlobalLayoutListener;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.WindowManager.LayoutParams;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.util.DisplayMetrics;

import java.util.Set;

import org.minidns.hla.DnssecResolverApi;
import org.minidns.hla.ResolverResult;
import org.minidns.record.SRV;
import org.minidns.record.TXT;

public class SuperTuxKartActivity extends SDLActivity
{
    private AlertDialog m_progress_dialog;
    private ProgressBar m_progress_bar;
    private STKEditText m_stk_edittext;
    private int m_bottom_y;
    // ------------------------------------------------------------------------
    private native void saveKeyboardHeight(int height);
    // ------------------------------------------------------------------------
    private native void saveMovedHeight(int height);
    // ------------------------------------------------------------------------
    private native static void addDNSSrvRecords(String name, int weight);
    // ------------------------------------------------------------------------
    private void showExtractProgressPrivate()
    {
        WindowManager wm =
            (WindowManager)getSystemService(Context.WINDOW_SERVICE);
        DisplayMetrics display_metrics = new DisplayMetrics();
        wm.getDefaultDisplay().getMetrics(display_metrics);
        int padding = display_metrics.widthPixels / 64;

        LinearLayout ll = new LinearLayout(this);
        ll.setOrientation(LinearLayout.VERTICAL);
        ll.setPadding(padding, padding, padding, padding);
        ll.setGravity(Gravity.CENTER);

        LinearLayout.LayoutParams ll_param = new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT,
            ViewGroup.LayoutParams.WRAP_CONTENT);
        ll_param.gravity = Gravity.CENTER;
        TextView tv = new TextView(this);
        tv.setText("Extracting game data...");
        tv.setLayoutParams(ll_param);

        ll_param = new LinearLayout.LayoutParams(
            display_metrics.widthPixels,
            LinearLayout.LayoutParams.WRAP_CONTENT);
        ll_param.gravity = Gravity.CENTER;
        ll.setLayoutParams(ll_param);

        m_progress_bar = new ProgressBar(this, null,
            android.R.attr.progressBarStyleHorizontal);
        m_progress_bar.setIndeterminate(false);
        m_progress_bar.setPadding(0, padding, 0, padding);
        m_progress_bar.setLayoutParams(ll_param);
        ll.addView(tv);
        ll.addView(m_progress_bar);

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setCancelable(false);
        builder.setView(ll);

        m_progress_dialog = builder.create();
        m_progress_dialog.show();
        Window window = m_progress_dialog.getWindow();
        if (window != null)
        {
            WindowManager.LayoutParams layout_params =
                new WindowManager.LayoutParams();
            layout_params.copyFrom(
                m_progress_dialog.getWindow().getAttributes());
            layout_params.width = WindowManager.LayoutParams.MATCH_PARENT;
            layout_params.height = LinearLayout.LayoutParams.WRAP_CONTENT;
            m_progress_dialog.getWindow().setAttributes(layout_params);
        }
    }
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
        m_progress_dialog = null;
        m_progress_bar = null;
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
                    saveMovedHeight(-moved_height);
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
    // ------------------------------------------------------------------------
    public void showExtractProgress(final int progress)
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                if (progress == -1)
                {
                    if (m_progress_dialog != null)
                    {
                        m_progress_dialog.dismiss();
                        m_progress_dialog = null;
                        m_progress_bar = null;
                    }
                    AlertDialog.Builder error =
                        new AlertDialog.Builder(SDL.getContext());
                    error.setMessage("Check remaining device space or " +
                        "reinstall SuperTuxKart.");
                    error.setTitle("Extract game data error");
                    error.setPositiveButton("Exit",
                        new DialogInterface.OnClickListener()
                        {
                            @Override
                            public void onClick(DialogInterface dialog,
                                                int id)
                            {
                                android.os.Process.killProcess(
                                    android.os.Process.myPid());
                            }
                        });
                    error.setCancelable(false);
                    error.create().show();
                    return;
                }
                if (progress == 0 && m_progress_dialog == null)
                    showExtractProgressPrivate();
                else if (progress == 100 && m_progress_dialog != null)
                {
                    m_progress_dialog.dismiss();
                    m_progress_dialog = null;
                    m_progress_bar = null;
                }
                else if (m_progress_bar != null &&
                    m_progress_bar.getProgress() != progress)
                {
                    m_progress_bar.setProgress(progress);
                }
            }
        });

    }
}
