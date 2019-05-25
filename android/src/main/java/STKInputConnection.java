package org.supertuxkart.stk_dbg;

import org.supertuxkart.stk_dbg.STKEditText;

import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputConnectionWrapper;

public class STKInputConnection extends InputConnectionWrapper
{
    /* The global edittext which will be "copied" to the current focused STK
     * box. */
    final private STKEditText m_stk_edittext;

    // ------------------------------------------------------------------------
    public STKInputConnection(InputConnection target, STKEditText stk_edittext)
    {
        super(target, true/*mutable*/);
        m_stk_edittext = stk_edittext;
    }
    // ------------------------------------------------------------------------
    @Override
    public boolean setComposingText(CharSequence text, int new_cursor_position)
    {
        boolean ret = super.setComposingText(text, new_cursor_position);
        String composing_text = text.toString();
        String new_text = m_stk_edittext.getText().toString();
        int composing_start = 0;
        int composing_end = 0;
        // Test last char
        if (!composing_text.isEmpty() && !new_text.isEmpty() &&
            composing_text.charAt(composing_text.length() - 1) ==
            new_text.charAt(new_text.length() - 1))
        {
            composing_start = new_text.length() - composing_text.length();
            composing_end = composing_start + composing_text.length();
        }
        m_stk_edittext.setComposingRegion(composing_start, composing_end);
        m_stk_edittext.updateSTKEditBox();
        return ret;
    }
    // ------------------------------------------------------------------------
    @Override
    public boolean finishComposingText()
    {
        m_stk_edittext.setComposingRegion(0, 0);
        m_stk_edittext.updateSTKEditBox();
        return super.finishComposingText();
    }
    // ------------------------------------------------------------------------
    @Override
    public boolean setComposingRegion(int start, int end)
    {
        m_stk_edittext.setComposingRegion(start, end);
        m_stk_edittext.updateSTKEditBox();
        return super.setComposingRegion(start, end);
    }
    // ------------------------------------------------------------------------
    @Override
    public boolean commitText(CharSequence text, int new_cursor_position)
    {
        // Usually only a single character, so dismiss composing region
        boolean ret = super.commitText(text, new_cursor_position);
        m_stk_edittext.setComposingRegion(0, 0);
        m_stk_edittext.updateSTKEditBox();
        return ret;
    }

}
