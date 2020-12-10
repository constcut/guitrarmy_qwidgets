package in.guitarmy.app;

import android.content.pm.ActivityInfo;
import android.app.Activity;
import android.app.AlertDialog;

import android.app.Notification;
import android.app.NotificationManager;
import android.content.Context;

//import android.media.midi.MidiManager;

import android.content.DialogInterface;

public class AndroidHelper extends org.qtproject.qt5.android.bindings.QtActivity
{
     private static AndroidHelper m_instance;

     private static Notification.Builder m_builder;
     private static NotificationManager m_notificationManager;

     public AndroidHelper()
     {
         m_instance = this; //useless but well..

     }

     public static void notify(String s)
     {
         if (m_notificationManager == null) {
             m_notificationManager = (NotificationManager)m_instance.getSystemService(Context.NOTIFICATION_SERVICE);
             m_builder = new Notification.Builder(m_instance);
             m_builder.setSmallIcon(R.drawable.icon);
             m_builder.setContentTitle("Guitarmy");
         }

         m_builder.setContentText(s);
         m_notificationManager.notify(1, m_builder.getNotification());
     }

     //min sdk 16

     public void setPortOrt()
     {
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
     }

     public static void msg()
     {
         /*
        AlertDialog.Builder builder = new AlertDialog.Builder(m_instance);
            builder.setTitle("Someth")
                    .setMessage("Is done")
                    .setCancelable(false)
                    .setNegativeButton("Ok",
                            new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int id) {
                                    dialog.cancel();
                                }
                            });
        AlertDialog alert = builder.create();
        alert.show();

        */
        //MidiManager midiMan =  (MidiManager) getSystemService(Context.MIDI_SERVICE);;
        ///MidiDeviceInfo[] infos = midiMan.getDevices();

        //MidiInputPort inputPort = device.openInputPort(index);
     }

     public void setLandOrt()
     {
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
     }

}
