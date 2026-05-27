package fastglob;

import fastcore.FastCore;

/**
 * FastGLOB Main API Class.
 * Native Windows capabilities exposed via JNI for high-performance globbing.
 */
public class FastGLOB {

    // Load the native library once upon class initialization
    static {
        FastCore.loadLibrary("fastglob");
    }

    /**
     * Traverses the specified base directory and returns all file paths matching the glob pattern.
     * 
     * @param baseDir The root directory to start searching
     * @param pattern The glob pattern
     * @return Array of matching relative file paths.
     */
    public static native String[] glob(String baseDir, String pattern);

}
