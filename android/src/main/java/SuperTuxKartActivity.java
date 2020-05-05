package org.supertuxkart.stk_dbg;

import org.supertuxkart.stk_dbg.STKEditText;

import org.libsdl.app.SDLControllerManager;
import org.libsdl.app.HIDDeviceManager;

import android.app.NativeActivity;
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

public class SuperTuxKartActivity extends NativeActivity
{
    // Same as irrlicht
    enum TouchInputEvent
    {
        ETIE_PRESSED_DOWN,
        ETIE_LEFT_UP,
        ETIE_MOVED,
        ETIE_COUNT
    }

    class TouchEventData
    {
        public TouchInputEvent event_type;
        public int x;
        public int y;
        TouchEventData()
        {
            event_type = TouchInputEvent.ETIE_PRESSED_DOWN;
            x = 0;
            y = 0;
        }
    };

    private TouchEventData[] m_touch_event_data;
    private STKEditText m_stk_edittext;
    private static Context m_context;
    private static HIDDeviceManager m_hid_device_manager;

    // ------------------------------------------------------------------------
    private native void saveKeyboardHeight(int height);
    // ------------------------------------------------------------------------
    private native void startSTK();
    // ------------------------------------------------------------------------
    private native static void addKey(int key_code, int action, int meta_state,
                                      int scan_code, int unichar);
    // ------------------------------------------------------------------------
    private native static void addTouch(int id, int x, int y, int event_type);
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
                if (!isHardwareKeyboardConnected() && m_stk_edittext != null)
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
        // We don't use native activity input event queue
        getWindow().takeInputQueue(null);
        System.loadLibrary("main");
        m_touch_event_data = new TouchEventData[32];
        for (int i = 0; i < 32; i++)
            m_touch_event_data[i] = new TouchEventData();
        m_context = this;
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
        SDLControllerManager.nativeSetupJNI();
        SDLControllerManager.initialize();
        m_hid_device_manager = HIDDeviceManager.acquire(this);
        // We only start stk main thread after JNI_OnLoad (for initializing the
        // java environment)
        startSTK();
    }
    // ------------------------------------------------------------------------
    @Override
    public void onPause()
    {
        super.onPause();
        hideKeyboardNative(false/*clear_text*/);
        if (m_hid_device_manager != null)
            m_hid_device_manager.setFrozen(true);
    }
    // ------------------------------------------------------------------------
    @Override
    public void onResume()
    {
        super.onResume();
        if (m_hid_device_manager != null)
            m_hid_device_manager.setFrozen(false);
    }
    // ------------------------------------------------------------------------
    @Override
    protected void onDestroy()
    {
        if (m_hid_device_manager != null)
        {
            HIDDeviceManager.release(m_hid_device_manager);
            m_hid_device_manager = null;
        }
        super.onDestroy();
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
        int key_code = event.getKeyCode();
        int action = event.getAction();
        int device_id = event.getDeviceId();
        int source = event.getSource();
        int meta_state = event.getMetaState();
        int scan_code = event.getScanCode();
        // KeyCharacterMap.COMBINING_ACCENT is not handled at the moment
        int unichar = event.getUnicodeChar(meta_state);
        int repeat_count = event.getRepeatCount();

        // Dispatch the different events depending on where they come from
        // Some SOURCE_JOYSTICK, SOURCE_DPAD or SOURCE_GAMEPAD are also SOURCE_KEYBOARD
        // So, we try to process them as JOYSTICK/DPAD/GAMEPAD events first, if that fails we try them as KEYBOARD
        //
        // Furthermore, it's possible a game controller has SOURCE_KEYBOARD and
        // SOURCE_JOYSTICK, while its key events arrive from the keyboard source
        // So, retrieve the device itself and check all of its sources
        if (SDLControllerManager.isDeviceSDLJoystick(device_id))
        {
            // Note that we process events with specific key codes here
            if (action == KeyEvent.ACTION_DOWN)
            {
                if (SDLControllerManager.onNativePadDown(device_id, key_code, meta_state, scan_code, unichar, repeat_count) == 0)
                    return true;
            }
            else if (action == KeyEvent.ACTION_UP)
            {
                if (SDLControllerManager.onNativePadUp(device_id, key_code, meta_state, scan_code, unichar) == 0)
                    return true;
            }
        }

        // User pressed back button
        if (key_code == KeyEvent.KEYCODE_BACK &&
            action == KeyEvent.ACTION_DOWN)
        {
            // Avoid repeating the event which leads to some strange behaviour
            // For ACTION_UP it's always zero
            if (repeat_count == 0)
                addKey(key_code, action, meta_state, scan_code, 0);
            return true;
        }

        boolean has_hardware_keyboard = isHardwareKeyboardConnected();
        // Allow hacker keyboard (one of the native android keyboard)
        // to close itself when pressing the esc button
        if (key_code == KeyEvent.KEYCODE_ESCAPE &&
            action == KeyEvent.ACTION_DOWN && !has_hardware_keyboard &&
            m_stk_edittext != null && m_stk_edittext.isFocused())
        {
            hideKeyboardNative(/*clear_text*/false);
            return true;
        }

        // Called when user change cursor / select all text in native android
        // keyboard
        boolean ret = super.dispatchKeyEvent(event);
        if (!has_hardware_keyboard && m_stk_edittext != null)
            m_stk_edittext.updateSTKEditBox();

        // Allow sending navigation key event found in native android keyboard
        // so user can using up or down to toggle multiple textfield
        boolean send_navigation = key_code == KeyEvent.KEYCODE_DPAD_UP ||
            key_code == KeyEvent.KEYCODE_DPAD_DOWN ||
            key_code == KeyEvent.KEYCODE_DPAD_LEFT ||
            key_code == KeyEvent.KEYCODE_DPAD_RIGHT;
        // We don't send navigation event if it's consumed by input method
        send_navigation = send_navigation && !ret;

        if (has_hardware_keyboard || send_navigation)
        {
            if (has_hardware_keyboard &&
                m_stk_edittext != null && m_stk_edittext.isFocused())
                m_stk_edittext.beforeHideKeyboard(/*clear_text*/true);
            addKey(key_code, action, meta_state, scan_code, unichar);
            // We use only java to handle key for hardware keyboard
            return true;
        }
        else
            return ret;
    }
    // ------------------------------------------------------------------------
    @Override
    public boolean dispatchGenericMotionEvent(MotionEvent ev)
    {
        if (SDLControllerManager.useGenericMotionListener(ev))
            return true;
        return super.dispatchGenericMotionEvent(ev);
    }
    // ------------------------------------------------------------------------
    @Override
    public boolean dispatchTouchEvent(MotionEvent ev)
    {
        boolean ret = super.dispatchTouchEvent(ev);
        if (!ret)
        {
            TouchInputEvent event_type;
            int action = ev.getAction();
            switch (action & MotionEvent.ACTION_MASK)
            {
            case MotionEvent.ACTION_DOWN:
            case MotionEvent.ACTION_POINTER_DOWN:
                event_type = TouchInputEvent.ETIE_PRESSED_DOWN;
                break;
            case MotionEvent.ACTION_MOVE:
                event_type = TouchInputEvent.ETIE_MOVED;
                break;
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_POINTER_UP:
            case MotionEvent.ACTION_CANCEL:
                event_type = TouchInputEvent.ETIE_LEFT_UP;
                break;
            default:
                return false;
            }

            int count = 1;
            int idx = (action & MotionEvent.ACTION_POINTER_INDEX_MASK) >>
                MotionEvent.ACTION_POINTER_INDEX_SHIFT;

            // Process all touches for move action.
            if (event_type == TouchInputEvent.ETIE_MOVED)
            {
                count = ev.getPointerCount();
                idx = 0;
            }

            for (int i = 0; i < count; i++)
            {
                int id = ev.getPointerId(i + idx);
                int x = (int)ev.getX(i + idx);
                int y = (int)ev.getY(i + idx);

                if (id >= 32)
                    continue;

                // Don't send move event when nothing changed
                if (m_touch_event_data[id].event_type == event_type &&
                    m_touch_event_data[id].x == x &&
                    m_touch_event_data[id].y == y)
                    continue;

                m_touch_event_data[id].event_type = event_type;
                m_touch_event_data[id].x = x;
                m_touch_event_data[id].y = y;
                addTouch(id, x, y, event_type.ordinal());
            }
            return true;
        }
        return false;
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
    public String[] getDNSSrvRecords(String domain)
    {
        try
        {
            ResolverResult<SRV> srvs =
                DnssecResolverApi.INSTANCE.resolve(domain, SRV.class);
            Set<SRV> ans = srvs.getAnswers();
            String[] result = new String[ans.size()];
            int i = 0;
            for (SRV s : ans)
                result[i++] = s.target.toString() + ":" + s.port;
            return result;
        }
        catch (Exception e)
        {
            e.printStackTrace();
            return new String[0];
        }
    }
    // ------------------------------------------------------------------------
    public boolean isHardwareKeyboardConnected()
    {
        return getResources().getConfiguration()
            .keyboard == Configuration.KEYBOARD_QWERTY;
    }
    // ------------------------------------------------------------------------
    public static Context getContext()                    { return m_context; }
}
