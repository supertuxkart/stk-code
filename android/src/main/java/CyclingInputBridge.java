package org.supertuxkart.stk_dbg;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.graphics.Typeface;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.os.ParcelUuid;
import android.os.SystemClock;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.TextView;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

import org.libsdl.app.SDLActivity;

/**
 * Android bridge for standard BLE Cycling Speed and Cadence (CSC) and Fitness
 * Machine Service (FTMS) sensors. The native input layer owns cadence
 * calibration and kart speed mapping; this class only discovers devices,
 * decodes telemetry, and supports simple two-button handlebar controls.
 */
@SuppressLint("MissingPermission")
public class CyclingInputBridge
{
    public static final int PERMISSION_REQUEST_CODE = 4202;

    private static final int SOURCE_CADENCE = 0;
    private static final int SOURCE_WHEEL = 1;
    private static final float ZERO_RPM_TIMEOUT_SECONDS = 2.4f;

    private static final UUID SERVICE_CSC =
        UUID.fromString("00001816-0000-1000-8000-00805f9b34fb");
    private static final UUID CHAR_CSC_MEASUREMENT =
        UUID.fromString("00002a5b-0000-1000-8000-00805f9b34fb");
    private static final UUID SERVICE_FTMS =
        UUID.fromString("00001826-0000-1000-8000-00805f9b34fb");
    private static final UUID CHAR_INDOOR_BIKE_DATA =
        UUID.fromString("00002ad2-0000-1000-8000-00805f9b34fb");
    private static final UUID DESC_CCCD =
        UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");

    private final Activity m_activity;
    private final Handler m_handler = new Handler(Looper.getMainLooper());
    private TextView m_hud_overlay;
    private BluetoothGatt m_bluetooth_gatt;

    private boolean m_started = false;
    private boolean m_handlebar_controls_enabled = false;
    private boolean m_overlay_enabled = true;
    private boolean m_scanning = false;
    private volatile boolean m_connected = false;

    private int m_previous_crank_revolutions = -1;
    private int m_previous_crank_time = -1;
    private long m_previous_wheel_revolutions = -1L;
    private int m_previous_wheel_time = -1;
    private volatile float m_raw_sensor_rpm = 0.0f;
    private volatile int m_telemetry_source = SOURCE_CADENCE;
    private float m_smoothed_sensor_rpm = 0.0f;
    private float m_cadence_rpm = 0.0f;
    private volatile long m_last_event_ms = 0L;
    private long m_last_tick_ms = 0L;

    private boolean m_drift_key_sent = false;
    private boolean m_left_key_sent = false;
    private boolean m_right_key_sent = false;
    private boolean m_fire_key_sent = false;

    private boolean m_physical_left_held = false;
    private boolean m_physical_right_held = false;
    private long m_turn_started_ms = 0L;

    public CyclingInputBridge(Activity activity)
    {
        m_activity = activity;
    }

    public void start()
    {
        if (m_started)
            return;

        m_started = true;
        m_last_tick_ms = SystemClock.elapsedRealtime();
        if (m_overlay_enabled)
            attachHudOverlay();
        if (!hasRequiredPermissions())
            requestPermissions();
        else
            startBleScan();
        m_handler.post(m_tick_runnable);
        m_handler.postDelayed(m_scan_watchdog_runnable, 8000L);
    }

    public void stop()
    {
        m_started = false;
        m_handler.removeCallbacksAndMessages(null);
        stopBleScan();

        BluetoothGatt gatt = m_bluetooth_gatt;
        m_bluetooth_gatt = null;
        if (gatt != null)
        {
            try
            {
                gatt.disconnect();
                gatt.close();
            }
            catch (Exception ignored) {}
        }

        m_connected = false;
        resetTelemetry();
        releaseHandlebarKeys();
        try
        {
            SuperTuxKartActivity.updateCyclingTelemetry(
                0.0f, SOURCE_CADENCE, false);
        }
        catch (UnsatisfiedLinkError ignored) {}

        detachHudOverlay();
    }

    public void onPermissionsResult()
    {
        if (m_started && hasRequiredPermissions())
            startBleScan();
    }

    public void setHandlebarControlsEnabled(boolean enabled)
    {
        m_handlebar_controls_enabled = enabled;
        if (!enabled)
            releaseHandlebarKeys();
    }

    public void setOverlayEnabled(boolean enabled)
    {
        m_overlay_enabled = enabled;
        if (!m_started)
            return;
        if (enabled)
            attachHudOverlay();
        else
            detachHudOverlay();
    }

    /**
     * Maps two small handlebar controllers to steering. Pressing both buttons
     * fires an item; holding a direction while pedaling engages drift.
     */
    public boolean handleKeyEvent(KeyEvent event)
    {
        if (!m_started || !m_handlebar_controls_enabled)
            return false;

        int code = event.getKeyCode();
        boolean down = event.getAction() == KeyEvent.ACTION_DOWN;

        boolean left_input = code == KeyEvent.KEYCODE_DPAD_LEFT ||
            code == KeyEvent.KEYCODE_DPAD_UP ||
            code == KeyEvent.KEYCODE_DPAD_DOWN ||
            code == KeyEvent.KEYCODE_A || code == KeyEvent.KEYCODE_Q ||
            code == KeyEvent.KEYCODE_Z ||
            code == KeyEvent.KEYCODE_BUTTON_L1 ||
            code == KeyEvent.KEYCODE_BUTTON_L2;

        boolean right_input = code == KeyEvent.KEYCODE_DPAD_RIGHT ||
            code == KeyEvent.KEYCODE_D || code == KeyEvent.KEYCODE_E ||
            code == KeyEvent.KEYCODE_C ||
            code == KeyEvent.KEYCODE_BUTTON_R1 ||
            code == KeyEvent.KEYCODE_BUTTON_R2 ||
            code == KeyEvent.KEYCODE_BUTTON_A ||
            code == KeyEvent.KEYCODE_BUTTON_B ||
            code == KeyEvent.KEYCODE_BUTTON_X ||
            code == KeyEvent.KEYCODE_BUTTON_Y;

        if (!left_input && !right_input)
            return false;

        if (left_input)
        {
            if (down && !m_physical_left_held && !m_physical_right_held)
                m_turn_started_ms = SystemClock.elapsedRealtime();
            m_physical_left_held = down;
        }
        if (right_input)
        {
            if (down && !m_physical_right_held && !m_physical_left_held)
                m_turn_started_ms = SystemClock.elapsedRealtime();
            m_physical_right_held = down;
        }
        updateSteeringAndChordState();
        return true;
    }

    private void attachHudOverlay()
    {
        if (!m_overlay_enabled || m_hud_overlay != null)
            return;

        m_hud_overlay = new TextView(m_activity);
        m_hud_overlay.setTextColor(Color.parseColor("#38BDF8"));
        m_hud_overlay.setBackgroundColor(Color.argb(140, 15, 23, 42));
        m_hud_overlay.setPadding(24, 10, 24, 10);
        m_hud_overlay.setTextSize(14.0f);
        m_hud_overlay.setTypeface(Typeface.MONOSPACE, Typeface.BOLD);
        m_hud_overlay.setText("CYCLING: SCANNING...");

        FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(
            FrameLayout.LayoutParams.WRAP_CONTENT,
            FrameLayout.LayoutParams.WRAP_CONTENT,
            Gravity.TOP | Gravity.START);
        params.setMargins(32, 24, 0, 0);
        m_activity.addContentView(m_hud_overlay, params);
    }

    private void detachHudOverlay()
    {
        if (m_hud_overlay == null)
            return;
        ViewGroup parent = (ViewGroup)m_hud_overlay.getParent();
        if (parent != null)
            parent.removeView(m_hud_overlay);
        m_hud_overlay = null;
    }

    private void updateSteeringAndChordState()
    {
        boolean both = m_physical_left_held && m_physical_right_held;
        boolean left = m_physical_left_held && !both;
        boolean right = m_physical_right_held && !both;

        setNativeKey(KeyEvent.KEYCODE_DPAD_LEFT, left, m_left_key_sent);
        m_left_key_sent = left;
        setNativeKey(KeyEvent.KEYCODE_DPAD_RIGHT, right, m_right_key_sent);
        m_right_key_sent = right;
        setNativeKey(KeyEvent.KEYCODE_SPACE, both, m_fire_key_sent);
        m_fire_key_sent = both;
    }

    private void releaseHandlebarKeys()
    {
        setNativeKey(KeyEvent.KEYCODE_DPAD_LEFT, false, m_left_key_sent);
        setNativeKey(KeyEvent.KEYCODE_DPAD_RIGHT, false, m_right_key_sent);
        setNativeKey(KeyEvent.KEYCODE_SPACE, false, m_fire_key_sent);
        setNativeKey(KeyEvent.KEYCODE_V, false, m_drift_key_sent);
        m_left_key_sent = false;
        m_right_key_sent = false;
        m_fire_key_sent = false;
        m_drift_key_sent = false;
        m_physical_left_held = false;
        m_physical_right_held = false;
        m_turn_started_ms = 0L;
    }

    private final Runnable m_tick_runnable = new Runnable()
    {
        @Override
        public void run()
        {
            if (!m_started)
                return;

            long now_ms = SystemClock.elapsedRealtime();
            float dt = Math.min(0.25f,
                Math.max(0.001f, (now_ms - m_last_tick_ms) / 1000.0f));
            m_last_tick_ms = now_ms;

            float silence_seconds = (now_ms - m_last_event_ms) / 1000.0f;
            float target = m_raw_sensor_rpm;
            float zero_timeout = ZERO_RPM_TIMEOUT_SECONDS;
            if (m_raw_sensor_rpm > 1.0f)
            {
                // Low-cadence sensors may legitimately take several seconds
                // between revolutions. Conversely, wheel sensors often batch
                // several revolutions into one notification, so never use a
                // revolution period as a notification-rate decay curve.
                zero_timeout = Math.min(5.0f, Math.max(zero_timeout,
                    1.5f * 60.0f / m_raw_sensor_rpm));
            }
            if (!m_connected || m_last_event_ms == 0L ||
                silence_seconds > zero_timeout)
            {
                target = 0.0f;
            }

            float tau = target > m_smoothed_sensor_rpm ? 0.15f : 0.45f;
            float alpha = 1.0f - (float)Math.exp(-dt / tau);
            m_smoothed_sensor_rpm +=
                alpha * (target - m_smoothed_sensor_rpm);
            if (m_smoothed_sensor_rpm < 1.0f && target == 0.0f)
                m_smoothed_sensor_rpm = 0.0f;

            try
            {
                m_cadence_rpm = SuperTuxKartActivity.updateCyclingTelemetry(
                    m_smoothed_sensor_rpm, m_telemetry_source, m_connected);
            }
            catch (UnsatisfiedLinkError ignored)
            {
                m_cadence_rpm = 0.0f;
            }

            boolean turning =
                (m_physical_left_held ^ m_physical_right_held) &&
                m_turn_started_ms > 0L;
            boolean drift = m_connected && turning &&
                now_ms - m_turn_started_ms > 350L && m_cadence_rpm >= 5.0f;
            setNativeKey(KeyEvent.KEYCODE_V, drift, m_drift_key_sent);
            m_drift_key_sent = drift;

            if (m_hud_overlay != null)
            {
                if (m_connected)
                {
                    m_hud_overlay.setText("CYCLING: " +
                        Math.round(m_cadence_rpm) + " RPM");
                }
                else
                {
                    m_hud_overlay.setText(m_scanning ?
                        "CYCLING: SCANNING..." : "CYCLING: OFF");
                }
            }
            m_handler.postDelayed(this, 33L);
        }
    };

    private static void setNativeKey(int key_code, boolean down,
                                     boolean currently_down)
    {
        if (down == currently_down)
            return;
        try
        {
            if (down)
                SDLActivity.onNativeKeyDown(key_code);
            else
                SDLActivity.onNativeKeyUp(key_code);
        }
        catch (UnsatisfiedLinkError ignored) {}
    }

    private final Runnable m_scan_watchdog_runnable = new Runnable()
    {
        @Override
        public void run()
        {
            if (!m_started)
                return;
            if (!m_connected && hasRequiredPermissions())
            {
                stopBleScan();
                startBleScan();
            }
            m_handler.postDelayed(this, 8000L);
        }
    };

    private boolean hasRequiredPermissions()
    {
        if (Build.VERSION.SDK_INT >= 31)
        {
            return m_activity.checkSelfPermission(
                "android.permission.BLUETOOTH_SCAN") ==
                    PackageManager.PERMISSION_GRANTED &&
                m_activity.checkSelfPermission(
                "android.permission.BLUETOOTH_CONNECT") ==
                    PackageManager.PERMISSION_GRANTED;
        }
        if (Build.VERSION.SDK_INT >= 23)
        {
            return m_activity.checkSelfPermission(
                "android.permission.ACCESS_FINE_LOCATION") ==
                    PackageManager.PERMISSION_GRANTED;
        }
        return true;
    }

    private void requestPermissions()
    {
        if (Build.VERSION.SDK_INT >= 31)
        {
            m_activity.requestPermissions(new String[] {
                "android.permission.BLUETOOTH_SCAN",
                "android.permission.BLUETOOTH_CONNECT"
            }, PERMISSION_REQUEST_CODE);
        }
        else if (Build.VERSION.SDK_INT >= 23)
        {
            m_activity.requestPermissions(new String[] {
                "android.permission.ACCESS_FINE_LOCATION"
            }, PERMISSION_REQUEST_CODE);
        }
    }

    private void startBleScan()
    {
        if (!m_started || m_scanning || m_connected ||
            Build.VERSION.SDK_INT < 21 || !hasRequiredPermissions())
        {
            return;
        }

        BluetoothManager manager = (BluetoothManager)m_activity
            .getSystemService(Context.BLUETOOTH_SERVICE);
        if (manager == null || manager.getAdapter() == null ||
            !manager.getAdapter().isEnabled())
        {
            return;
        }
        BluetoothLeScanner scanner =
            manager.getAdapter().getBluetoothLeScanner();
        if (scanner == null)
            return;

        List<ScanFilter> filters = new ArrayList<>();
        filters.add(new ScanFilter.Builder()
            .setServiceUuid(new ParcelUuid(SERVICE_CSC)).build());
        filters.add(new ScanFilter.Builder()
            .setServiceUuid(new ParcelUuid(SERVICE_FTMS)).build());
        ScanSettings settings = new ScanSettings.Builder()
            .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY).build();
        try
        {
            scanner.startScan(filters, settings, m_scan_callback);
            m_scanning = true;
        }
        catch (Exception ignored) {}
    }

    private void stopBleScan()
    {
        if (!m_scanning || Build.VERSION.SDK_INT < 21)
            return;
        m_scanning = false;
        BluetoothManager manager = (BluetoothManager)m_activity
            .getSystemService(Context.BLUETOOTH_SERVICE);
        if (manager == null || manager.getAdapter() == null)
            return;
        BluetoothLeScanner scanner =
            manager.getAdapter().getBluetoothLeScanner();
        if (scanner != null)
        {
            try
            {
                scanner.stopScan(m_scan_callback);
            }
            catch (Exception ignored) {}
        }
    }

    private final ScanCallback m_scan_callback = new ScanCallback()
    {
        @Override
        public void onScanResult(int callback_type, ScanResult result)
        {
            if (!m_started || result == null || result.getDevice() == null ||
                m_connected)
            {
                return;
            }
            stopBleScan();
            BluetoothDevice device = result.getDevice();
            m_bluetooth_gatt = device.connectGatt(
                m_activity, false, m_gatt_callback);
        }
    };

    private final BluetoothGattCallback m_gatt_callback =
        new BluetoothGattCallback()
    {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status,
                                            int new_state)
        {
            if (!m_started)
            {
                try { gatt.close(); } catch (Exception ignored) {}
                return;
            }
            if (new_state == BluetoothProfile.STATE_CONNECTED)
            {
                m_connected = true;
                m_scanning = false;
                gatt.discoverServices();
            }
            else if (new_state == BluetoothProfile.STATE_DISCONNECTED)
            {
                m_connected = false;
                resetTelemetry();
                try { gatt.close(); } catch (Exception ignored) {}
                if (m_bluetooth_gatt == gatt)
                    m_bluetooth_gatt = null;
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status)
        {
            if (!m_started || status != BluetoothGatt.GATT_SUCCESS)
                return;

            BluetoothGattCharacteristic target = null;
            BluetoothGattService csc = gatt.getService(SERVICE_CSC);
            if (csc != null)
                target = csc.getCharacteristic(CHAR_CSC_MEASUREMENT);
            if (target == null)
            {
                BluetoothGattService ftms = gatt.getService(SERVICE_FTMS);
                if (ftms != null)
                    target = ftms.getCharacteristic(CHAR_INDOOR_BIKE_DATA);
            }
            if (target == null)
                return;

            gatt.setCharacteristicNotification(target, true);
            BluetoothGattDescriptor cccd = target.getDescriptor(DESC_CCCD);
            if (cccd != null)
            {
                cccd.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
                gatt.writeDescriptor(cccd);
            }
        }

        @Override
        @SuppressWarnings("deprecation")
        public void onCharacteristicChanged(BluetoothGatt gatt,
            BluetoothGattCharacteristic characteristic)
        {
            byte[] bytes = characteristic.getValue();
            if (!m_started || bytes == null || bytes.length == 0)
                return;
            long now_ms = SystemClock.elapsedRealtime();
            if (CHAR_CSC_MEASUREMENT.equals(characteristic.getUuid()))
                parseCsc(bytes, now_ms);
            else if (CHAR_INDOOR_BIKE_DATA.equals(characteristic.getUuid()))
                parseIndoorBikeData(bytes, now_ms);
        }
    };

    private void resetTelemetry()
    {
        m_previous_crank_revolutions = -1;
        m_previous_crank_time = -1;
        m_previous_wheel_revolutions = -1L;
        m_previous_wheel_time = -1;
        m_raw_sensor_rpm = 0.0f;
        m_smoothed_sensor_rpm = 0.0f;
        m_cadence_rpm = 0.0f;
        m_last_event_ms = 0L;
        m_telemetry_source = SOURCE_CADENCE;
    }

    private void parseIndoorBikeData(byte[] bytes, long now_ms)
    {
        if (bytes.length < 2)
            return;
        ByteBuffer buffer = ByteBuffer.wrap(bytes).order(ByteOrder.LITTLE_ENDIAN);
        int flags = buffer.getShort() & 0xffff;

        if ((flags & 0x0001) == 0 && buffer.remaining() >= 2)
            buffer.position(buffer.position() + 2); // instantaneous speed
        if ((flags & 0x0002) != 0 && buffer.remaining() >= 2)
            buffer.position(buffer.position() + 2); // average speed
        if ((flags & 0x0004) != 0 && buffer.remaining() >= 2)
        {
            float rpm = (buffer.getShort() & 0xffff) * 0.5f;
            if (rpm <= 220.0f)
                acceptTelemetry(rpm, SOURCE_CADENCE, now_ms);
        }
    }

    private void parseCsc(byte[] bytes, long now_ms)
    {
        ByteBuffer buffer = ByteBuffer.wrap(bytes).order(ByteOrder.LITTLE_ENDIAN);
        int flags = buffer.get() & 0xff;
        boolean wheel_present = (flags & 0x01) != 0;
        boolean crank_present = (flags & 0x02) != 0;

        if (wheel_present && buffer.remaining() >= 6)
        {
            long revolutions = buffer.getInt() & 0xffffffffL;
            int event_time = buffer.getShort() & 0xffff;
            if (!crank_present)
            {
                if (m_previous_wheel_revolutions != -1L &&
                    m_previous_wheel_time != -1)
                {
                    long delta_revolutions =
                        (revolutions - m_previous_wheel_revolutions) &
                        0xffffffffL;
                    int delta_ticks =
                        (event_time - m_previous_wheel_time) & 0xffff;
                    if (delta_ticks > 0 && delta_revolutions > 0 &&
                        delta_revolutions < 50)
                    {
                        float rpm = delta_revolutions * 61440.0f /
                            (float)delta_ticks;
                        if (rpm <= 1000.0f)
                            acceptTelemetry(rpm, SOURCE_WHEEL, now_ms);
                    }
                }
                else
                {
                    m_last_event_ms = now_ms;
                }
                m_previous_wheel_revolutions = revolutions;
                m_previous_wheel_time = event_time;
            }
        }

        if (crank_present && buffer.remaining() >= 4)
        {
            int revolutions = buffer.getShort() & 0xffff;
            int event_time = buffer.getShort() & 0xffff;
            if (m_previous_crank_revolutions != -1 &&
                m_previous_crank_time != -1)
            {
                int delta_revolutions =
                    (revolutions - m_previous_crank_revolutions) & 0xffff;
                int delta_ticks =
                    (event_time - m_previous_crank_time) & 0xffff;
                if (delta_ticks > 0 && delta_revolutions > 0 &&
                    delta_revolutions < 20)
                {
                    float rpm = delta_revolutions * 61440.0f /
                        (float)delta_ticks;
                    if (rpm <= 220.0f)
                        acceptTelemetry(rpm, SOURCE_CADENCE, now_ms);
                }
            }
            else
            {
                m_last_event_ms = now_ms;
            }
            m_previous_crank_revolutions = revolutions;
            m_previous_crank_time = event_time;
        }
    }

    private void acceptTelemetry(float rpm, int source, long now_ms)
    {
        m_raw_sensor_rpm = Math.max(0.0f, rpm);
        m_telemetry_source = source;
        m_last_event_ms = now_ms;
    }
}
