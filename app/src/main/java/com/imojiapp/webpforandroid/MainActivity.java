package com.imojiapp.webpforandroid;

import android.graphics.Bitmap;
import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.ImageView;

import com.imojiapp.bifrost.Bifrost;

import java.io.IOException;
import java.io.InputStream;
import java.nio.Buffer;
import java.nio.ByteBuffer;


public class MainActivity extends ActionBarActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //setContentView(R.layout.activity_main);

        try {
            InputStream is = getApplicationContext().getResources().openRawResource(R.raw.initial_head_1);
            Log.d("onCreate", is.available() + " bytes available.");
            byte[] b = new byte[is.available()];
            int bytesRead = is.read(b);
            Log.d("onCreate", bytesRead + " bytes read into array.");

            ByteBuffer in = ByteBuffer.allocateDirect(b.length);
            in.put(b);

            Bitmap bitmap = Bifrost.decode(in, 2048, 2048);

            ByteBuffer out = Bifrost.encode(bitmap, Bifrost.WEBP_PRESET_PICTURE, 10);
            Bitmap bitmap2 = Bifrost.decode(out, 2048, 2048);
            Bifrost.releaseBuffer(out);

            ImageView iv = new ImageView(getApplicationContext());
            iv.setImageBitmap(bitmap2);
            setContentView(iv);
        } catch(Throwable e) {
            throw new RuntimeException(e);
        }
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }
}
