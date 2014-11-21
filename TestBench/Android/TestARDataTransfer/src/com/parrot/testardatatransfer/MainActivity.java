/*
    Copyright (C) 2014 Parrot SA

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the 
      distribution.
    * Neither the name of Parrot nor the names
      of its contributors may be used to endorse or promote products
      derived from this software without specific prior written
      permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
    OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
    AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
    OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/
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

import com.parrot.arsdk.ardatatransfer.ARDATATRANSFER_DOWNLOADER_RESUME_ENUM;
import com.parrot.arsdk.ardatatransfer.ARDATATRANSFER_ERROR_ENUM;
import com.parrot.arsdk.ardatatransfer.ARDATATRANSFER_UPLOADER_RESUME_ENUM;
import com.parrot.arsdk.ardatatransfer.ARDataTransferDataDownloader;
import com.parrot.arsdk.ardatatransfer.ARDataTransferDataDownloaderFileCompletionListener;
import com.parrot.arsdk.ardatatransfer.ARDataTransferDownloader;
import com.parrot.arsdk.ardatatransfer.ARDataTransferDownloaderCompletionListener;
import com.parrot.arsdk.ardatatransfer.ARDataTransferDownloaderProgressListener;
import com.parrot.arsdk.ardatatransfer.ARDataTransferException;
import com.parrot.arsdk.ardatatransfer.ARDataTransferManager;
import com.parrot.arsdk.ardatatransfer.ARDataTransferMedia;
import com.parrot.arsdk.ardatatransfer.ARDataTransferMediasDownloader;
import com.parrot.arsdk.ardatatransfer.ARDataTransferMediasDownloaderAvailableMediaListener;
import com.parrot.arsdk.ardatatransfer.ARDataTransferMediasDownloaderCompletionListener;
import com.parrot.arsdk.ardatatransfer.ARDataTransferMediasDownloaderProgressListener;
import com.parrot.arsdk.ardatatransfer.ARDataTransferUploader;
import com.parrot.arsdk.ardatatransfer.ARDataTransferUploaderCompletionListener;
import com.parrot.arsdk.ardatatransfer.ARDataTransferUploaderProgressListener;
import com.parrot.arsdk.arutils.ARUtilsException;
import com.parrot.arsdk.arutils.ARUtilsFtpConnection;
import com.parrot.arsdk.arutils.ARUtilsManager;

public class MainActivity 
	extends Activity 
	implements
	    ARDataTransferDataDownloaderFileCompletionListener,
	    ARDataTransferMediasDownloaderProgressListener,
		ARDataTransferMediasDownloaderCompletionListener,
		ARDataTransferMediasDownloaderAvailableMediaListener,
		ARDataTransferDownloaderProgressListener,
		ARDataTransferDownloaderCompletionListener,
		ARDataTransferUploaderProgressListener,
		ARDataTransferUploaderCompletionListener
		
{
	public static String APP_TAG = "TestARDataTransfer "; 
        
	//public static String DRONE_IP = "172.20.5.146";
	public static String DRONE_IP = "172.20.5.28";
	public static int DRONE_PORT = 21;
	
	ARDataTransferManager managerRunning = null;
	ARUtilsManager utilsListDataManagerRunning = null;
	ARUtilsManager utilsDataManagerRunning = null;
	ARUtilsManager utilsListManagerRunning = null;
	ARUtilsManager utilsQueueManagerRunning = null;
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
		
		Button testDownloader = (Button)this.findViewById(R.id.testDownloader);
        
		testDownloader.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				TestARDataTransferDownloader();
			}
		});		

		Button testUploader = (Button)this.findViewById(R.id.testUploader);
        
		testUploader.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				TestARDataTransferUploader();
			}
		});
		
		TestDataDownloader();
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
        
        TestDataDownloader();
        //TestMediasDownloader();
    }
    
    public void TestDataDownloader()
    {
    	try
    	{
	    	ARDataTransferManager manager = new ARDataTransferManager();
	    	ARUtilsManager utilsListManager = new ARUtilsManager();
	    	ARUtilsManager utilsDataManager = new ARUtilsManager();
	    	
	    	utilsListManager.initWifiFtp(DRONE_IP, DRONE_PORT, ARUtilsFtpConnection.FTP_ANONYMOUS, "");
	    	utilsDataManager.initWifiFtp(DRONE_IP, DRONE_PORT, ARUtilsFtpConnection.FTP_ANONYMOUS, "");
	    	ARDataTransferDataDownloader dataManager = manager.getARDataTransferDataDownloader();
	    	
        	File sysHome = this.getFilesDir();// /data/data/com.example.tstdata/files
        	String tmp = sysHome.getAbsolutePath();
        	
        	dataManager.createDataDownloader(utilsListManager, utilsDataManager, "", tmp, this, this);
        	
        	long count = dataManager.getAvailableFiles();
        	Log.d("DBG", "getAvailableFiles " + count);
        	
        	Runnable dataDownloader = dataManager.getDownloaderRunnable();
        	
        	Thread dataThread = new Thread(dataDownloader);
        	dataThread.start();
        	
        	//dataManager.cancelThread();
	    	
        	try { dataThread.join(); } catch (InterruptedException e) { Log.d("DBG", "join " + e.toString());  }
        	
        	dataManager.dispose();
        	manager.dispose();
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
        	ARUtilsManager utilsListManager = new ARUtilsManager();
        	ARUtilsManager utilsQueueManager = new ARUtilsManager();
        	utilsListManager.initWifiFtp(DRONE_IP, DRONE_PORT, ARUtilsFtpConnection.FTP_ANONYMOUS, "");
        	utilsQueueManager.initWifiFtp(DRONE_IP, DRONE_PORT, ARUtilsFtpConnection.FTP_ANONYMOUS, "");
        	//String list = null;
        	
        	//list = manager.ftpEasyList("ftp://172.20.5.109/", "/");
        	//list = manager.ftpEasyList("ftp://192.168.1.1/", "boxes");
        	
        	ARDataTransferMediasDownloader mediasManager = manager.getARDataTransferMediasDownloader();        	
        	
        	String tmp = "/var";
        	File sysTmp = this.getCacheDir();// /data/data/com.example.tstdata/cache
        	tmp = sysTmp.getAbsolutePath();
        	
        	File sysHome = this.getFilesDir();// /data/data/com.example.tstdata/files
        	tmp = sysHome.getAbsolutePath();
        	
        	
        	mediasManager.createMediasDownloader(utilsListManager, utilsQueueManager, "", tmp);
        	
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
        		//Log.d("DBG", APP_TAG + "thumbnail: " + new String(media.getThumbnail()));
        		
        		mediasManager.addMediaToQueue(media, this, this, this, this);
        	}
        	
        	//mediasManager.cancelQueuedMedias();
        	
        	//Runnable mediasDownloader = mediasManager.getDownloaderQueueRunnable();
        	//new Thread(mediasDownloader).start();
        	
        	mediasManager.cancelQueueThread();
        	
        	try { mediasThread.join(); } catch (InterruptedException e) { Log.d("DBG", "join " + e.toString());  }
        	
        	mediasManager.dispose();
        	manager.dispose();
        	utilsListManager.closeWifiFtp();
        	utilsQueueManager.closeWifiFtp();
        	utilsListManager.dispose();
        	utilsQueueManager.dispose();
        	
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
	    	ARDataTransferManager manager = null;
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
	        
	        try
	        {
	        	manager = new ARDataTransferManager();
	        } catch (ARDataTransferException e) {
	        	Log.d("DBG", "createManager ERROR " + e.toString()); 
	    		assertError(e.getError() == ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_OK); 
	        }
	
	        //no manager
	    	manager.dispose();
	    	Log.d("DBG", "dispose");
	    	
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
	    		manager = new ARDataTransferManager();
	    		Log.d("DBG", "createManager OK"); 
	    	} catch (ARDataTransferException e) { 
	    		Log.d("DBG", "createManager ERROR " + e.toString()); 
	    		assertError(e.getError() == ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_OK); 
	    	}
	    	ARUtilsManager utilsListDataManager = null;
            try {
                utilsListDataManager = new ARUtilsManager();
            } catch (ARUtilsException e) {
                Log.d("DBG", "new ARUtilsManager failed");
                assertError(utilsListDataManager == null);
            }
            utilsListDataManager.initWifiFtp(DRONE_IP, DRONE_PORT, ARUtilsFtpConnection.FTP_ANONYMOUS, "");
            ARUtilsManager utilsDataManager = null;
            try {
                utilsDataManager = new ARUtilsManager();
            } catch (ARUtilsException e) {
                Log.d("DBG", "new ARUtilsManager failed");
                assertError(utilsDataManager == null);
            }
            utilsDataManager.initWifiFtp(DRONE_IP, DRONE_PORT, ARUtilsFtpConnection.FTP_ANONYMOUS, "");            
	    	
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
	        	dataManager.createDataDownloader(utilsListDataManager, utilsDataManager, "", tmp, this, this); 
	        	Log.d("DBG", "initialize OK"); 
	        } catch (ARDataTransferException e) { 
	        	Log.d("DBG", "initialize ERROR " + e.toString());
	        	assertError(e.getError() == ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_OK);
	        }
	        
	        try { 
	        	dataManager.createDataDownloader(utilsListDataManager, utilsDataManager, "", tmp, this, this); 
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
	        ARUtilsManager utilsListManager = null;
	        ARUtilsManager utilsQueueManager = null;
	        try {
				utilsListManager = new ARUtilsManager();
			} catch (ARUtilsException e) {
				Log.d("DBG", "new ARUtilsManager failed");
				assertError(utilsListManager == null);
			}
	        utilsListManager.initWifiFtp(DRONE_IP, DRONE_PORT, ARUtilsFtpConnection.FTP_ANONYMOUS, "");
	        try {
                utilsQueueManager = new ARUtilsManager();
            } catch (ARUtilsException e) {
                Log.d("DBG", "new ARUtilsManager failed");
                assertError(utilsQueueManager == null);
            }
            utilsQueueManager.initWifiFtp(DRONE_IP, DRONE_PORT, ARUtilsFtpConnection.FTP_ANONYMOUS, "");
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
	        	mediasManager.createMediasDownloader(utilsListManager, utilsQueueManager, "", tmp); 
	        	Log.d("DBG", "initialize OK"); 
	        } catch (ARDataTransferException e) { 
	        	Log.d("DBG", "initialize ERROR " + e.toString());
	        	assertError(e.getError() == ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_OK);
	        }
	        
	        try {
	        	mediasManager.createMediasDownloader(utilsListManager, utilsQueueManager, "", tmp); 
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
	        
	        dataManager.dispose();
	        mediasManager.dispose();
	        manager.dispose();
	        utilsListDataManager.closeWifiFtp();
	        utilsDataManager.closeWifiFtp();
	        utilsListManager.closeWifiFtp();
	        utilsQueueManager.closeWifiFtp();
	        utilsListDataManager.dispose();
	        utilsDataManager.dispose();
	        utilsListManager.dispose();
	        utilsQueueManager.dispose();
	        Log.d("DBG", "dispose OK");
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
            utilsListDataManagerRunning = new ARUtilsManager();
            utilsListDataManagerRunning.initWifiFtp(DRONE_IP, DRONE_PORT, ARUtilsFtpConnection.FTP_ANONYMOUS, "");            
            utilsDataManagerRunning = new ARUtilsManager();
            utilsDataManagerRunning.initWifiFtp(DRONE_IP, DRONE_PORT, ARUtilsFtpConnection.FTP_ANONYMOUS, "");
            utilsListManagerRunning = new ARUtilsManager();
            utilsListManagerRunning.initWifiFtp(DRONE_IP, DRONE_PORT, ARUtilsFtpConnection.FTP_ANONYMOUS, "");
            utilsQueueManagerRunning = new ARUtilsManager();
            utilsQueueManagerRunning.initWifiFtp(DRONE_IP, DRONE_PORT, ARUtilsFtpConnection.FTP_ANONYMOUS, "");

            semRunning = new Semaphore(1);
        	
        	semRunning.acquire();
            File sysHome = this.getFilesDir();// /data/data/com.example.tstdata/files
            String tmp = sysHome.getAbsolutePath();
        
            //Data
            ARDataTransferDataDownloader dataManager = managerRunning.getARDataTransferDataDownloader();
            dataManager.createDataDownloader(utilsListDataManagerRunning, utilsDataManagerRunning, "", tmp, this, this);
            
            Runnable dataDownloader = dataManager.getDownloaderRunnable();
            Thread dataThread = new Thread(dataDownloader);
            dataThread.start();
            
            //Media
            ARDataTransferMediasDownloader mediasManager = managerRunning.getARDataTransferMediasDownloader();        	
            mediasManager.createMediasDownloader(utilsListManagerRunning, utilsQueueManagerRunning, "", tmp);
            
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
            
	        dataManager.dispose();
	        mediasManager.dispose();
            managerRunning.dispose();
            utilsListDataManagerRunning.closeWifiFtp();
            utilsDataManagerRunning.closeWifiFtp();
            utilsListManagerRunning.closeWifiFtp();
	        utilsQueueManagerRunning.closeWifiFtp();
	        utilsListDataManagerRunning.dispose();
	        utilsDataManagerRunning.dispose();
	        utilsListManagerRunning.dispose();
            utilsQueueManagerRunning.dispose();
        }
        catch (ARDataTransferException e) 
        {
        	Log.d("DBG", APP_TAG + e.toString()); 
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
    		media.getName() /*+ ", " + media.getFilePath()*/ +  ", " + media.getProduct().toString() + ", " + media.getDate() + ", " + media.getUUID());
    }
    
    public void TestARDataTransferAvailableMedia()
    {
    	try
    	{
	    	ARDataTransferManager manager = new ARDataTransferManager();
	    	ARUtilsManager utilsListManager = new ARUtilsManager();
	    	utilsListManager.initWifiFtp(DRONE_IP, DRONE_PORT, ARUtilsFtpConnection.FTP_ANONYMOUS, "");
	    	ARUtilsManager utilsQueueManager = new ARUtilsManager();
            utilsQueueManager.initWifiFtp(DRONE_IP, DRONE_PORT, ARUtilsFtpConnection.FTP_ANONYMOUS, "");
	    	
	    	ARDataTransferMediasDownloader mediasManager = manager.getARDataTransferMediasDownloader();        	
	    	
	    	String tmp = "/var";
	    	File sysTmp = this.getCacheDir();// /data/data/com.example.tstdata/cache
	    	tmp = sysTmp.getAbsolutePath();
	    	
	    	File sysHome = this.getFilesDir();// /data/data/com.example.tstdata/files
	    	tmp = sysHome.getAbsolutePath();
	    	
	    	mediasManager.createMediasDownloader(utilsListManager, utilsQueueManager, "", tmp);
	    	
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
    
    void TestARDataTransferDownloader()
    {
    	try
    	{
    		ARDataTransferManager manager = new ARDataTransferManager();
	    	ARUtilsManager utilsManager = new ARUtilsManager();
	    	utilsManager.initWifiFtp(DRONE_IP, DRONE_PORT, ARUtilsFtpConnection.FTP_ANONYMOUS, "");
	    	
	    	ARDataTransferDownloader downloadManager = manager.getARDataTransferDownloader();        	
	    	
	    	String tmp = "/var";
	    	File sysTmp = this.getCacheDir();// /data/data/com.example.tstdata/cache
	    	tmp = sysTmp.getAbsolutePath();
	    	
	    	File sysHome = this.getFilesDir();// /data/data/com.example.tstdata/files
	    	tmp = sysHome.getAbsolutePath();
	    	String name = "Bebop_Drone_2014-01-21T160101+0100_3902B87F947BE865A9D137CFA63492B8.jpg";
	    	String remoteName =  "Bebop_Drone/media/" + name;
	    	
	    	downloadManager.createDownloader(utilsManager, remoteName, tmp + "/" + name, this, this, this, this, ARDATATRANSFER_DOWNLOADER_RESUME_ENUM.ARDATATRANSFER_DOWNLOADER_RESUME_FALSE);
	    	
	    	Runnable downloader = downloadManager.getDownloaderRunnable();
	    	
	    	Thread downloaderThread = new Thread(downloader);
	    	downloaderThread.start();
	    	
	    	try { downloaderThread.join(); } catch (InterruptedException e) { Log.d("DBG", "join " + e.toString());  }
            
	        downloadManager.dispose();
            manager.dispose();
            utilsManager.closeWifiFtp();
	        utilsManager.dispose();
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
    
    void TestARDataTransferUploader()
    {
    	try
    	{
    		ARDataTransferManager manager = new ARDataTransferManager();
	    	ARUtilsManager utilsManager = new ARUtilsManager();
	    	utilsManager.initWifiFtp(DRONE_IP, DRONE_PORT, ARUtilsFtpConnection.FTP_ANONYMOUS, "");
	    	
	    	ARDataTransferUploader uploadManager = manager.getARDataTransferUploader();        	
	    	
	    	String tmp = "/var";
	    	File sysTmp = this.getCacheDir();// /data/data/com.example.tstdata/cache
	    	tmp = sysTmp.getAbsolutePath();
	    	
	    	File sysHome = this.getFilesDir();// /data/data/com.example.tstdata/files
	    	tmp = sysHome.getAbsolutePath();
	    	String name = "Bebop_Drone_2014-01-21T160101+0100_3902B87F947BE865A9D137CFA63492B8.jpg";
	    	String remoteName =  "Bebop_Drone/media/" + name + ".new";
    		
	    	uploadManager.createUploader(utilsManager, remoteName, tmp + "/" + name , this, this, this, this, ARDATATRANSFER_UPLOADER_RESUME_ENUM.ARDATATRANSFER_UPLOADER_RESUME_FALSE);
	    	
	    	Runnable uploader = uploadManager.getUploaderRunnable();
	    	
	    	Thread uploaderThread = new Thread(uploader);
	    	uploaderThread.start();
	    	
	    	try { uploaderThread.join(); } catch (InterruptedException e) { Log.d("DBG", "join " + e.toString());  }
            
	        uploadManager.dispose();
            manager.dispose();
            utilsManager.closeWifiFtp();
	        utilsManager.dispose();
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
    
    @Override
    public void didDataDownloaderFileComplete(Object arg, String fileName, ARDATATRANSFER_ERROR_ENUM error)
    {
        Log.d("DBG", APP_TAG + "ARDataTransferDataDownloader, didDataDownloaderFileComplete: " + fileName);
    }
    
    public void didMediaProgress(Object arg, ARDataTransferMedia media, float percent)
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
    
    public void didDownloadProgress(Object arg, float percent)
    {
    	Log.d("DBG", APP_TAG + "ARDataTransferDownloader, didDownloadProgress: " + percent + "%");
    }
    
    public void didDownloadComplete(Object arg, ARDATATRANSFER_ERROR_ENUM error)
    {
    	String err;
    	if (error == ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_OK)
    		err = "ARDATATRANSFER_OK";
    	else
    		err = "[" + error.toString() + "]";
    	Log.d("DBG", APP_TAG + "ARDataTransferDownloader, didDownloadComplete: " + err);    	
    }
    
    public void didUploadProgress(Object arg, float percent)
    {
    	Log.d("DBG", APP_TAG + "ARDataTransferDownloader, didUploadProgress: " + percent + "%");
    }
    
    public void didUploadComplete(Object arg, ARDATATRANSFER_ERROR_ENUM error)
    {
    	String err;
    	if (error == ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_OK)
    		err = "ARDATATRANSFER_OK";
    	else
    		err = "[" + error.toString() + "]";
    	Log.d("DBG", APP_TAG + "ARDataTransferDownloader, didUploadComplete: " + err);
    }
}
