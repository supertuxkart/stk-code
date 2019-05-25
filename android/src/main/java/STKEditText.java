package org.supertuxkart.stk_dbg;

import org.supertuxkart.stk_dbg.STKInputConnection;

import android.content.Context;
import android.text.InputType;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.View;
import android.widget.EditText;

// We need to extend EditText instead of view to allow copying to our STK
// editbox
public class STKEditText extends EditText
{
    private int m_composing_start;

    private int m_composing_end;

    // ------------------------------------------------------------------------
    private native static void editText2STKEditbox(String full_text, int start,
                                                  int end, int composing_start,
                                                  int composing_end);
    // ------------------------------------------------------------------------
    public STKEditText(Context context)
    {
        super(context);
        setFocusableInTouchMode(true);
        m_composing_start = 0;
        m_composing_end = 0;
    }
    // ------------------------------------------------------------------------
    @Override
    public InputConnection onCreateInputConnection(EditorInfo out_attrs)
    {
        STKInputConnection sic =
            new STKInputConnection(super.onCreateInputConnection(out_attrs), this);
        out_attrs.actionLabel = null;
        out_attrs.inputType = InputType.TYPE_CLASS_TEXT;
        out_attrs.imeOptions = EditorInfo.IME_ACTION_NEXT |
            EditorInfo.IME_FLAG_NO_FULLSCREEN |
            EditorInfo.IME_FLAG_NO_EXTRACT_UI;
        return sic;
    }
    // ------------------------------------------------------------------------
    @Override
    public boolean onCheckIsTextEditor()                       { return true; }
    // ------------------------------------------------------------------------
    public void resetWhenFocus()
    {
        clearComposingText();
        getText().clear();
        m_composing_start = m_composing_end = 0;
    }
    // ------------------------------------------------------------------------
    public void setComposingRegion(int start, int end)
    {
        // From doc of InputConnectionWrapper, it says:
        // Editor authors, be ready to accept a start that is greater than end.
        if (start != end && start > end)
        {
            m_composing_end = start;
            m_composing_start = end;
        }
        else
        {
            m_composing_start = start;
            m_composing_end = end;
        }
    }
    // ------------------------------------------------------------------------
    public void updateSTKEditBox()
    {
        if (!isFocused())
            return;
        editText2STKEditbox(getText().toString(), getSelectionStart(),
            getSelectionEnd(), m_composing_start, m_composing_end);
    }
    // ------------------------------------------------------------------------
    public void beforeHideKeyboard()
    {
        clearFocus();
        setVisibility(View.GONE);
    }
}
