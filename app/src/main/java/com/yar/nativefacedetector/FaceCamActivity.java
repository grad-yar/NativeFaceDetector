package com.yar.nativefacedetector;

import android.Manifest;
import android.content.Context;
import android.content.DialogInterface;
import android.content.pm.PackageManager;
import android.hardware.Camera;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.widget.Toast;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.JavaCameraView;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.android.Utils;
import org.opencv.core.Mat;
import org.opencv.core.MatOfRect;
import org.opencv.core.Point;
import org.opencv.core.Rect;
import org.opencv.core.Scalar;
import org.opencv.imgproc.Imgproc;

import java.io.File;

public class FaceCamActivity extends AppCompatActivity {

    public static final String TAG = FaceCamActivity.class.getSimpleName();
    public static final int CAMERA_PERMISSION_REQUEST_CODE = 42;

    private OpenCVLoaderCallback mOpenCVLoaderCallback;

    private JavaCameraView mJavaCameraView;

    private DetectionBasedTracker mNativeTracker;

    private float mRelativeFaceSize = 0.2f;
    private int mAbsoluteFaceSize = 0;
    private Scalar mFaceRectColor = new Scalar(0, 255, 0, 255);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_face_cam);

        mJavaCameraView = (JavaCameraView) findViewById(R.id.j_cam_view);

        mJavaCameraView.setVisibility(JavaCameraView.VISIBLE);
        mJavaCameraView.setCvCameraViewListener(new CamViewListener());

        mOpenCVLoaderCallback = new OpenCVLoaderCallback(this);
    }

    @Override
    public void onResume() {
        super.onResume();

        if (deviceHasCameras()) {
            if (appHasCameraPermission()) {
                initOpenCV();
            } else {
                requestPermissionForCamera();
            }
        } else {
            showDialogNoCamera();
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        if (mJavaCameraView != null)
            mJavaCameraView.disableView();
    }

    @Override
    protected void onStop() {
        super.onStop();
        if (mJavaCameraView != null)
            mJavaCameraView.disableView();
        if (mNativeTracker != null)
            mNativeTracker.release();
    }

    private class CamViewListener implements CameraBridgeViewBase.CvCameraViewListener2 {

        Point center;

        @Override
        public void onCameraViewStarted(int width, int height) {
            if (mAbsoluteFaceSize == 0) {
                if (Math.round(height * mRelativeFaceSize) > 0) {
                    mAbsoluteFaceSize = Math.round(height * mRelativeFaceSize);
                }
                mNativeTracker.setMinFaceSize(mAbsoluteFaceSize);
            }

            center = new Point(width / 2, height / 2);

            mNativeTracker.start();
        }

        @Override
        public void onCameraViewStopped() {
            mNativeTracker.stop();
        }

        @Override
        public Mat onCameraFrame(CameraBridgeViewBase.CvCameraViewFrame inputFrame) {

            Mat rgba = inputFrame.rgba();
            Mat gray = inputFrame.gray();

/*
            Mat rotationMatrix2D = Imgproc.getRotationMatrix2D(center, 90, 1.0);

            Imgproc.warpAffine(rgba, rgba, rotationMatrix2D, rgba.size());
            Imgproc.warpAffine(gray, gray, rotationMatrix2D, gray.size());

*/

            MatOfRect faces = new MatOfRect();

            mNativeTracker.detect(gray, faces);

            Rect[] facesArray = faces.toArray();
            for (int i = 0; i < facesArray.length; i++)
                Imgproc.rectangle(rgba, facesArray[i].tl(), facesArray[i].br(), mFaceRectColor, 4);

            return rgba;
        }

    }

    private class OpenCVLoaderCallback extends BaseLoaderCallback {

        public OpenCVLoaderCallback(Context AppContext) {
            super(AppContext);
        }

        @Override
        public void onManagerConnected(int status) {
            if (status == LoaderCallbackInterface.SUCCESS) {
                Log.i(TAG, "OpenCV loaded successfully");

                String cascadeFile = Utils.exportResource(
                        FaceCamActivity.this, R.raw.haarcascade_frontalface_default);

                mNativeTracker = new DetectionBasedTracker(cascadeFile, 0);

                new File(cascadeFile).delete();

                mJavaCameraView.enableView();

            } else {
                super.onManagerConnected(status);
            }
        }

    }

    @SuppressWarnings("deprecation")
    private boolean deviceHasCameras() {
        int numberOfCameras = Camera.getNumberOfCameras();

        if (numberOfCameras > 0) {

            int frontCameraIndex = -1;

            for (int i = 0; i < numberOfCameras; i++) {
                Camera.CameraInfo cameraInfo = new Camera.CameraInfo();
                Camera.getCameraInfo(i, cameraInfo);
                if (cameraInfo.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
                    frontCameraIndex = i;
                    break;
                }
            }

            if (frontCameraIndex > -1) {
                mJavaCameraView.setCameraIndex(frontCameraIndex);
            } else {
                Toast.makeText(this, R.string.text_front_cam_not_found, Toast.LENGTH_LONG).show();
                mJavaCameraView.setCameraIndex(0);
            }

            return true;
        }

        return false;
    }

    private void initOpenCV() {
        if (!OpenCVLoader.initDebug()) {
            Log.d(TAG, "Internal OpenCV library not found. Using OpenCV Manager for initialization");
            OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_3_1_0, this, mOpenCVLoaderCallback);
        } else {
            Log.d(TAG, "OpenCV library found inside package. Using it!");
            mOpenCVLoaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS);
        }
    }

    private boolean appHasCameraPermission() {
        int result = ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA);
        return result == PackageManager.PERMISSION_GRANTED;
    }

    public void requestPermissionForCamera() {
        if (ActivityCompat.shouldShowRequestPermissionRationale(this, Manifest.permission.CAMERA)) {
            showDialogPermissionRequired();
        } else {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.CAMERA}, CAMERA_PERMISSION_REQUEST_CODE);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);

        switch (requestCode) {
            case CAMERA_PERMISSION_REQUEST_CODE: {
                if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    initOpenCV();
                } else {
                    requestPermissionForCamera();
                }
            }
        }
    }

    private void showDialogPermissionRequired() {
        AlertDialog.Builder adb = new AlertDialog.Builder(this)
                .setTitle(R.string.dialog_title_permission_required)
                .setMessage(R.string.text_cam_permission_explanation)
                .setCancelable(false)
                .setPositiveButton(R.string.text_btn_ok, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        dialogInterface.dismiss();
                        finish();
                    }
                });
        adb.show();
    }

    private void showDialogNoCamera() {
        AlertDialog.Builder adb = new AlertDialog.Builder(this)
                .setTitle(R.string.dialog_title_no_camera)
                .setMessage(R.string.dialog_message_no_camera_installed)
                .setCancelable(false)
                .setPositiveButton(R.string.text_btn_ok, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        dialogInterface.dismiss();
                        finish();
                    }
                });
        adb.show();
    }
}
