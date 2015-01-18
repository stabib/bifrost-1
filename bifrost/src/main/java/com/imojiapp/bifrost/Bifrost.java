/**
 * Created by thor on 16/01/15.
 */

package com.imojiapp.bifrost;

import android.graphics.Bitmap;
import java.nio.ByteBuffer;

public class Bifrost {
    static {
        System.loadLibrary("webp");
    }

    public static final int WEBP_PRESET_DEFAULT = 0; // default preset.
    public static final int WEBP_PRESET_PICTURE = 1; // digital picture, like portrait, inner shot
    public static final int WEBP_PRESET_PHOTO = 2;   // outdoor photograph, with natural lighting
    public static final int WEBP_PRESET_DRAWING = 3; // hand or line drawing, with high-contrast details
    public static final int WEBP_PRESET_ICON = 4;    // small-sized colorful images
    public static final int WEBP_PRESET_TEXT = 5;    // text-like

    /*
     * Decode ByteBuffer into Bitmap while limiting dimensions and preserving aspect ratio
     *  NOTE: The buffer _must_ be a direct byte buffer!
     */
    public static native Bitmap decode(ByteBuffer in, int maxWidth, int maxHeight);

    /* Encode Bitmap into ByteBuffer with given preset and quality (0-100)
     * NOTE: The bitmap _must_ be in ARGB_8888 pixel format!
     * NOTE: Always call releaseBuffer() after you're done with the buffer!
     */
    public static native ByteBuffer encode(Bitmap bitmap, int preset, float quality);

    /* Release the native memory backing the ByteBuffer returned by encode()
     * NOTE: Accessing the ByteBuffer object after this may cause your program to crash!
     */
    public static native void releaseBuffer(ByteBuffer buffer);
}
