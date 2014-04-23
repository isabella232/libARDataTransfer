package com.parrot.testardatatransfer;

import java.io.File;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

import com.parrot.arsdk.ardatatransfer.ARDATATRANSFER_ERROR_ENUM;
import com.parrot.arsdk.ardatatransfer.ARDataTransferDataDownloader;
import com.parrot.arsdk.ardatatransfer.ARDataTransferException;
import com.parrot.arsdk.ardatatransfer.ARDataTransferManager;
import com.parrot.arsdk.ardatatransfer.ARDataTransferMedia;
import com.parrot.arsdk.ardatatransfer.ARDataTransferMediasDownloader;
import com.parrot.arsdk.ardatatransfer.ARDataTransferMediasDownloaderAvailableMediaListener;
import com.parrot.arsdk.ardatatransfer.ARDataTransferMediasDownloaderCompletionListener;
import com.parrot.arsdk.ardatatransfer.ARDataTransferMediasDownloaderProgressListener;

public class MainActivity 
	extends Activity 
	implements ARDataTransferMediasDownloaderProgressListener, 
		ARDataTransferMediasDownloaderCompletionListener,
		ARDataTransferMediasDownloaderAvailableMediaListener
{
	public static String APP_TAG = "TestARDataTransfer "; 
        
	public static String DRONE_IP = "172.20.5.146";
	//public static String DRONE_IP = "192.168.1.1";
	public static int DRONE_PORT = 21;
	
	ARDataTransferManager managerRunning = null;
	Semaphore semRunning = null;
        
    @SuppressWarnings("serial")
	public class TestException extends Exception
    {
    	public TestException()
    	{
    		super("\n=============== ASSERT TestException ==============");
    	}
    }
        
    public void assertError(boolean status) throws TestException
	{
    	if (false == status)
    	{
    		throw new TestException();
    	}
	}        

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
        Log.d("DBG", APP_TAG + "onCreate");
        
        //LoadModules(true);
        LoadModules(false);
        
        Button test = (Button)this.findViewById(R.id.testJni);
        
        test.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				TestJni();				
				//TestARDataTransferParameters();
			}
		});
		
		Button testRunning = (Button)this.findViewById(R.id.testRunning);
        
		testRunning.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				
				new Thread(new Runnable() {
					@Override
					public void run() {
						TestARDataTransferParameters();
						TestARDataTransferRunning();
					}
				}).start();
			}
		});
		
		Button testAvailable = (Button)this.findViewById(R.id.testAvailable);
		
		testAvailable.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				
				new Thread(new Runnable() {
					@Override
					public void run() {
						TestARDataTransferAvailableMedia();
					}
				}).start();
			}
		});
		
		Button testRunningSignal = (Button)this.findViewById(R.id.testRunningSignal);
        
		testRunningSignal.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				TestARDataTransferRunningSignal();
			}
		});		
	}
	
	@Override
	protected void onPause()
	{
		super.onPause();
		TestARDataTransferRunningSignal();
	}

	@Override
	protected void onStop()
	{
		super.onStop();
		TestARDataTransferRunningSignal();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}
    
    private void LoadModules(boolean debug)
    {
    	try
        {
    		//debug = false;
    		if (debug == false)
    		{
    			System.loadLibrary("curl");
    			System.loadLibrary("arsal");
    			System.loadLibrary("arsal_android");
    			System.loadLibrary("arutils");
    			System.loadLibrary("arutils_android");
    			System.loadLibrary("ardiscovery");
    			System.loadLibrary("ardiscovery_android");
        		System.loadLibrary("ardatatransfer");
        		System.loadLibrary("ardatatransfer_android");
    		}
    		else
    		{
    			System.loadLibrary("curl");

    			System.loadLibrary("arsal_dbg");
    			System.loadLibrary("arsal_android_dbg");
    			System.loadLibrary("arutils_dbg");
    			System.loadLibrary("arutils_android_dbg");
    			System.loadLibrary("ardiscovery_dbg");
    			System.loadLibrary("ardiscovery_android_dbg");
        		System.loadLibrary("ardatatransfer_dbg");
        		System.loadLibrary("ardatatransfer_android_dbg");
    		}
        }
        catch (Exception e)
        {
        	Log.d("DBG", APP_TAG + e.toString());
        }
        catch (Throwable e)
        {
        	Log.d("DBG", APP_TAG + e.toString());
        }
    }
    
    private void TestJni()
    {
        Log.d("DBG", APP_TAG + "TestJni");
        
        //LoadModules(true);
        //LoadModules(false);
        
        //TestDataDownloader();
        TestMediasDownloader();
    }
    
    private void TestDataDownloader()
    {
    	try
    	{
	    	ARDataTransferManager manager = new ARDataTransferManager();

	    	manager.createManager();
	    	
	    	ARDataTransferDataDownloader dataManager = manager.getARDataTransferDataDownloader();
	    	
        	File sysHome = this.getFilesDir();// /data/data/com.example.tstdata/files
        	String tmp = sysHome.getAbsolutePath();
        	
        	dataManager.createDataDownloader(DRONE_IP, DRONE_PORT, tmp);
        	
        	Runnable dataDownloader = dataManager.getDownloaderRunnable();
        	
        	Thread dataThread = new Thread(dataDownloader);
        	dataThread.start();
        	
        	//dataManager.cancelThread();
	    	
        	try { dataThread.join(); } catch (InterruptedException e) { Log.d("DBG", "join " + e.toString());  }
        	
        	dataManager.closeDataDownloader();
        	manager.closeManager();
    	 }
        catch (Exception e)
        {
        	Log.d("DBG", e.toString());
        }
        catch (Throwable e)
        {
        	Log.d("DBG", e.toString());
        }	    	
    }
    
    private void TestMediasDownloader()
    {
        try
        {
        	ARDataTransferManager manager = new ARDataTransferManager();
        	manager.createManager();
        	//String list = null;
        	
        	//list = manager.ftpEasyList("ftp://172.20.5.109/", "/");
        	//list = manager.ftpEasyList("ftp://192.168.1.1/", "boxes");
        	
        	ARDataTransferMediasDownloader mediasManager = manager.getARDataTransferMediasDownloader();        	
        	
        	String tmp = "/var";
        	File sysTmp = this.getCacheDir();// /data/data/com.example.tstdata/cache
        	tmp = sysTmp.getAbsolutePath();
        	
        	File sysHome = this.getFilesDir();// /data/data/com.example.tstdata/files
        	tmp = sysHome.getAbsolutePath();
        	
        	mediasManager.createMediasDownloader(DRONE_IP, DRONE_PORT, tmp);
        	
        	Runnable mediasDownloader = mediasManager.getDownloaderQueueRunnable();
        	Thread mediasThread = new Thread(mediasDownloader);
        	mediasThread.start();
        	
        	//mediasManager.cancelQueueThread();
        	
        	int count = mediasManager.getAvailableMediasSync(true);
        	
        	mediasManager.getAvailableMediasAsync(this, this);
        	
        	for (int i=0; i<count; i++)
        	{
        		ARDataTransferMedia media = mediasManager.getAvailableMediaAtIndex(i);
        		Log.d("DBG", APP_TAG + "media name:" + media.getName() + ", date: " + media.getDate() + ", size: " + media.getSize() + ", thumbnail: " + media.getThumbnail().length);
        		Log.d("DBG", APP_TAG + "thumbnail: " + new String(media.getThumbnail()));
        		
        		mediasManager.addMediaToQueue(media, this, this, this, this);
        	}
        	
        	//mediasManager.cancelQueuedMedias();
        	
        	//Runnable mediasDownloader = mediasManager.getDownloaderQueueRunnable();
        	//new Thread(mediasDownloader).start();
        	
        	mediasManager.cancelQueueThread();
        	
        	try { mediasThread.join(); } catch (InterruptedException e) { Log.d("DBG", "join " + e.toString());  }
        	
        	mediasManager.closeMediasDownloader();
        	manager.closeManager();
        	
        	//Log.d("DBG", APP_TAG + list);
        	//TextView text = (TextView)this.findViewById(R.id.text_filed);
        	//if (null != list) text.setText(list);
        }
        catch (Exception e)
        {
        	Log.d("DBG", APP_TAG + e.toString());
        }
        catch (Throwable e)
        {
        	Log.d("DBG", APP_TAG + e.toString());
        }
    }
        
    public void TestARDataTransferParameters()
    {
    	try
    	{
	    	ARDataTransferManager manager = new ARDataTransferManager();
	    	ARDataTransferDataDownloader dataManager = null;
	    	ARDataTransferMediasDownloader mediasManager = null;
	    	Runnable dataDownloader = null;
	    	Runnable mediasDownloader = null;
	    	Thread dataThread = null;
	    	Thread mediasThread = null;
	    	int mediasCount = 0;
	    	ARDATATRANSFER_ERROR_ENUM result = ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_OK;
	    	
	    	File sysHome = this.getFilesDir();// /data/data/com.example.tstdata/files
	        String tmp = sysHome.getAbsolutePath();
	
	        //no manager
	    	manager.closeManager();
	    	Log.d("DBG", "closeManager");
	    	
	    	boolean isInit = manager.isInitialized();
	    	Log.d("DBG", "isInitialized " + (isInit == false ? "OK" : "ERROR"));
	    	assertError(isInit == false);
	    	
	    	dataManager = manager.getARDataTransferDataDownloader();
	    	Log.d("DBG", "getARDataTransferDataDownloader " + (dataManager == null ? "OK" : "ERROR"));
	    	assertError(dataManager == null);
	    	
	    	mediasManager = manager.getARDataTransferMediasDownloader();
	    	Log.d("DBG", "getARDataTransferMediasDownloader " + (mediasManager == null ? "OK" : "ERROR"));
	    	assertError(mediasManager == null);
	    	
	    	//not initialized
	    	try { 
	    		manager.createManager(); 
	    		Log.d("DBG", "createManager OK"); 
	    	} catch (ARDataTransferException e) { 
	    		Log.d("DBG", "createManager ERROR " + e.toString()); 
	    		assertError(e.getError() == ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_OK); 
	    	}
	    	
	    	dataManager = manager.getARDataTransferDataDownloader();
	    	Log.d("DBG", "getARDataTransferDataDownloader " + (dataManager != null ? "OK" : "ERROR"));
	    	assertError(dataManager != null);
	    	
	        dataDownloader = dataManager.getDownloaderRunnable();
	        Log.d("DBG", "getDownloaderRunnable " + (dataDownloader == null ? "OK" : "ERROR"));
	        assertError(dataDownloader == null);
	        
	        mediasManager = manager.getARDataTransferMediasDownloader();
	        Log.d("DBG", "getARDataTransferMediasDownloader " + (mediasManager != null ? "OK" : "ERROR"));
	        assertError(mediasManager != null);
	            	
	        mediasDownloader = mediasManager.getDownloaderQueueRunnable();
	        Log.d("DBG", "getDownloaderQueueRunnable " + (mediasManager == null ? "OK" : "ERROR"));
	        assertError(mediasDownloader == null);
	
	        //Data
	        dataManager = manager.getARDataTransferDataDownloader();
	    	Log.d("DBG", "getARDataTransferDataDownloader " + (dataManager != null ? "OK" : "ERROR"));
	    	assertError(dataManager != null);
	    	
	        result = dataManager.cancelThread();
	        Log.d("DBG", "cancelThread " + (result == ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_ERROR_NOT_INITIALIZED ? "OK" : "ERROR"));
	        assertError(result == ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_ERROR_NOT_INITIALIZED);	    	
	        
	        try { 
	        	dataManager.createDataDownloader(DRONE_IP, DRONE_PORT, tmp); 
	        	Log.d("DBG", "initialize OK"); 
	        } catch (ARDataTransferException e) { 
	        	Log.d("DBG", "initialize ERROR " + e.toString());
	        	assertError(e.getError() == ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_OK);
	        }
	        
	        try { 
	        	dataManager.createDataDownloader(DRONE_IP, DRONE_PORT, tmp); 
	        	Log.d("DBG", "initialize ERROR"); 
	        } catch (ARDataTransferException e) { 
	        	Log.d("DBG", "initialize " + (e.getError() == ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_ERROR_ALREADY_INITIALIZED ? "OK" : "ERROR"));
	        	assertError(e.getError() == ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_ERROR_ALREADY_INITIALIZED);
	        }
	        
	        dataDownloader = dataManager.getDownloaderRunnable();
	        Log.d("DBG", "getDownloaderRunnable " + (dataDownloader != null ? "OK" : "ERROR"));
	        assertError(dataDownloader != null);
	        
	        dataThread = new Thread(dataDownloader);
	        dataThread.start();
	        Log.d("DBG", "start OK");
	        
	        result = dataManager.cancelThread();
	        Log.d("DBG", "cancelThread " + (result == ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_OK ? "OK" : "ERROR"));
	        assertError(result == ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_OK);
	        
	        //Medias
	        mediasManager = manager.getARDataTransferMediasDownloader();
	        Log.d("DBG", "getARDataTransferMediasDownloader " + (mediasManager != null ? "OK" : "ERROR"));
	        assertError(mediasManager != null);
       
	        result = mediasManager.cancelQueueThread();
	        Log.d("DBG", "cancelQueueThread " + (result == ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_ERROR_NOT_INITIALIZED ? "OK" : "ERROR"));
	        assertError(result == ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_ERROR_NOT_INITIALIZED);
	        
	        try { 
	        	mediasCount = mediasManager.getAvailableMediasSync(true);
	        	Log.d("DBG", "getAvailableMedias ERROR"); 
	        } catch (ARDataTransferException e) { 
	        	Log.d("DBG", "getAvailableMedias " + (e.getError() == ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_ERROR_NOT_INITIALIZED ? "OK" : "ERROR"));
	        	assertError(e.getError() == ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_ERROR_NOT_INITIALIZED);
	        }
	        
	        try { 
	        	mediasManager.addMediaToQueue(null, null, null, null, null); 
	        	Log.d("DBG", "addMediaToQueue ERROR"); 
	        } catch (ARDataTransferException e) { 
	        	Log.d("DBG", "addMediaToQueue " + (e.getError() == ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_ERROR_BAD_PARAMETER ? "OK" : "ERROR"));
	        	assertError(e.getError() == ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_ERROR_BAD_PARAMETER);
	        }
	        
	        try {
	        	mediasManager.createMediasDownloader(DRONE_IP, DRONE_PORT, tmp); 
	        	Log.d("DBG", "initialize OK"); 
	        } catch (ARDataTransferException e) { 
	        	Log.d("DBG", "initialize ERROR " + e.toString());
	        	assertError(e.getError() == ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_OK);
	        }
	        
	        try {
	        	mediasManager.createMediasDownloader(DRONE_IP, DRONE_PORT, tmp); 
	        	Log.d("DBG", "initialize ERROR"); 
	        } catch (ARDataTransferException e) { 
	        	Log.d("DBG", "initialize " + (e.getError() == ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_ERROR_ALREADY_INITIALIZED ? "OK" : "ERROR"));
	        	assertError(e.getError() == ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_ERROR_ALREADY_INITIALIZED);
	        }
	        	        
	        try {
	        	mediasCount = mediasManager.getAvailableMediasSync(true);
	        	Log.d("DBG", "getAvailableMedias OK"); 
	        } catch (ARDataTransferException e) { 
	        	Log.d("DBG", "getAvailableMedias " + e.toString());
	        	assertError(e.getError() == ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_OK);
	        }
	        
	        try {
	        	mediasManager.addMediaToQueue(0 != mediasCount ? mediasManager.getAvailableMediaAtIndex(0) : null, null, null, null, null);
	        	
		        Log.d("DBG", "addMediaToQueue OK");
	        } catch (ARDataTransferException e) { 
	        	Log.d("DBG", "addMediaToQueue " + e.toString());
	        	assertError(e.getError() == ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_OK);
	        }
	        
	        mediasDownloader = mediasManager.getDownloaderQueueRunnable();
	        Log.d("DBG", "getDownloaderQueueRunnable " + (mediasManager != null ? "OK" : "ERROR"));
	        assertError(mediasDownloader != null);
	        
	        mediasThread = new Thread(mediasDownloader);
	        mediasThread.start();
	        Log.d("DBG", "start OK");
	        
	        result = mediasManager.cancelQueueThread();
	        Log.d("DBG", "cancelQueueThread " + (result == ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_OK ? "OK" : "ERROR"));
	        assertError(result == ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_OK);
	        
	        try { dataThread.join(); } catch (InterruptedException e) { Log.d("DBG", "join " + e.toString());  }
	        try { mediasThread.join(); } catch (InterruptedException e) { Log.d("DBG", "join " + e.toString());  }
	        
	        dataManager.closeDataDownloader();
	        mediasManager.closeMediasDownloader();
	        manager.closeManager();
	        Log.d("DBG", "closeManager OK");
    	}
    	catch (TestException e)
    	{
    		Log.d("DBG", "ERROR EXIT");
    	}
    }
    
    public void TestARDataTransferRunning()
    {
        try
        {
            managerRunning = new ARDataTransferManager();
            semRunning = new Semaphore(1);
        	
        	semRunning.acquire();
            File sysHome = this.getFilesDir();// /data/data/com.example.tstdata/files
            String tmp = sysHome.getAbsolutePath();
        	
            managerRunning.createManager();
        
            //Data
            ARDataTransferDataDownloader dataManager = managerRunning.getARDataTransferDataDownloader();
            dataManager.createDataDownloader(DRONE_IP, DRONE_PORT, tmp);
            
            Runnable dataDownloader = dataManager.getDownloaderRunnable();
            Thread dataThread = new Thread(dataDownloader);
            dataThread.start();
            
            //Media
            ARDataTransferMediasDownloader mediasManager = managerRunning.getARDataTransferMediasDownloader();        	
            mediasManager.createMediasDownloader(DRONE_IP, DRONE_PORT, tmp);
            
            Runnable mediasDownloader = mediasManager.getDownloaderQueueRunnable();
            Thread mediasThread = new Thread(mediasDownloader);
            mediasThread.start();
            
            do
            {
            	int count = mediasManager.getAvailableMediasSync(true);
        	
            	for (int i=0; i<count; i++)
				{
					ARDataTransferMedia media = mediasManager.getAvailableMediaAtIndex(i);
					Log.d("DBG", APP_TAG + "media name:" + media.getName() + ", date: " + media.getDate() + ", size: " + media.getSize() + ", thumbnail: " + media.getThumbnail().length);
					//Log.d("DBG", APP_TAG + "thumbnail: " + new String(media.getThumbnail()));
				
					mediasManager.addMediaToQueue(media, this, this, this, this);
					
					//ARDATATRANSFER_ERROR_ENUM error = mediasManager.deleteMedia(media);
					//Log.d("DBG", APP_TAG + error.toString());
				}
            }
            while (false == semRunning.tryAcquire(20, TimeUnit.SECONDS));
            
	        try { dataThread.join(); } catch (InterruptedException e) { Log.d("DBG", "join " + e.toString());  }
	        try { mediasThread.join(); } catch (InterruptedException e) { Log.d("DBG", "join " + e.toString());  }
            
	        dataManager.closeDataDownloader();
	        mediasManager.closeMediasDownloader();
            managerRunning.closeManager();
        }
        catch (Exception e)
        {
        	Log.d("DBG", APP_TAG + e.toString());
        }
        catch (Throwable e)
        {
        	Log.d("DBG", APP_TAG + e.toString());
        }
    }
    
    public void didMediaAvailable(Object arg, ARDataTransferMedia media, int index)
    {
    	Log.d("DBG", APP_TAG + "ARDataTransferMediasDownloader, didMediaAvailable: "+ index + " " +
    		media.getName() /*+ ", " + media.getFilePath()*/ +  ", " + media.getProduct().toString());
    }
    
    public void TestARDataTransferAvailableMedia()
    {
    	try
    	{
	    	ARDataTransferManager manager = new ARDataTransferManager();
	    	manager.createManager();
	    	
	    	ARDataTransferMediasDownloader mediasManager = manager.getARDataTransferMediasDownloader();        	
	    	
	    	String tmp = "/var";
	    	File sysTmp = this.getCacheDir();// /data/data/com.example.tstdata/cache
	    	tmp = sysTmp.getAbsolutePath();
	    	
	    	File sysHome = this.getFilesDir();// /data/data/com.example.tstdata/files
	    	tmp = sysHome.getAbsolutePath();
	    	
	    	mediasManager.createMediasDownloader(DRONE_IP, DRONE_PORT, tmp);
	    	
	    	mediasManager.getAvailableMediasAsync(this, this);
    	}
        catch (Exception e)
        {
        	Log.d("DBG", APP_TAG + e.toString());
        }
        catch (Throwable e)
        {
        	Log.d("DBG", APP_TAG + e.toString());
        }
    }

    public void TestARDataTransferRunningSignal()
    {
    	try
    	{
    		if (null != semRunning)
    		{
    			semRunning.release();
    		}
    		
    		if (null != managerRunning)
    		{
    			managerRunning.getARDataTransferDataDownloader().cancelThread();
    			managerRunning.getARDataTransferMediasDownloader().cancelQueueThread();
    		}
    	}
        catch (Exception e)
        {
        	Log.d("DBG", APP_TAG + e.toString());
        }
        catch (Throwable e)
        {
        	Log.d("DBG", APP_TAG + e.toString());
        }
    }
    
    public void didMediaProgress(Object arg, ARDataTransferMedia media, int percent)
    {
    	Log.d("DBG", APP_TAG + "ARDataTransferMediasDownloader, didMediaProgress: " + media.getName() + ", " + percent + "%");
    }
    
    public void didMediaComplete(Object arg, ARDataTransferMedia media, ARDATATRANSFER_ERROR_ENUM error)
    {
    	String err;
    	if (error == ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_OK)
    		err = "ARDATATRANSFER_OK";
    	else
    		err = "[" + error.toString() + "]";
    	Log.d("DBG", APP_TAG + "ARDataTransferMediasDownloader, didMediaComplete: " + media.getName() + ", " + err);
    }
}
