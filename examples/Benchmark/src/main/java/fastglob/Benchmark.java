package fastglob;

import java.io.File;
import java.io.IOException;
import java.nio.file.FileSystems;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.PathMatcher;
import java.nio.file.Paths;
import java.util.stream.Stream;

/**
 * Performance Benchmark comparing FastGLOB to standard Java alternatives.
 */
public class Benchmark {
    public static void main(String[] args) throws IOException {
        System.out.println("=== FastGLOB Performance Benchmark ===");
        
        // Go up to the workspace root to traverse all projects (huge amount of files!)
        Path searchRoot = Paths.get("..").toAbsolutePath().normalize();
        String pattern = "**/*.java";
        
        System.out.println("Traversing root: " + searchRoot);
        System.out.println("Glob Pattern: " + pattern);
        
        // Warmup
        for (int i = 0; i < 3; i++) {
            FastGLOB.glob(searchRoot.toString(), pattern);
            runJavaGlob(searchRoot, "**/*.java");
        }
        
        int runs = 5;
        System.out.println("\nRunning " + runs + " iterations...");
        
        // 1. Benchmark Standard Java NIO
        long startJava = System.nanoTime();
        int javaCount = 0;
        for (int i = 0; i < runs; i++) {
            javaCount = runJavaGlob(searchRoot, "**/*.java");
        }
        long javaTimeMs = (System.nanoTime() - startJava) / 1_000_000 / runs;
        
        // 2. Benchmark FastGLOB Native
        long startNative = System.nanoTime();
        int nativeCount = 0;
        for (int i = 0; i < runs; i++) {
            String[] matches = FastGLOB.glob(searchRoot.toString(), pattern);
            nativeCount = matches.length;
        }
        long nativeTimeMs = (System.nanoTime() - startNative) / 1_000_000 / runs;
        
        System.out.println("\n=== Results ===");
        System.out.println("Standard Java Matches: " + javaCount + " in " + javaTimeMs + " ms (avg)");
        System.out.println("FastGLOB Matches:      " + nativeCount + " in " + nativeTimeMs + " ms (avg)");
        
        if (nativeTimeMs > 0) {
            float speedup = (float) javaTimeMs / nativeTimeMs;
            System.out.printf("Speedup:               %.2fx faster! 🚀\n", speedup);
        } else {
            System.out.println("Speedup:               Infinite (Native was too fast to measure) 🚀");
        }
    }
    
    private static int runJavaGlob(Path root, String globPattern) throws IOException {
        PathMatcher matcher = FileSystems.getDefault().getPathMatcher("glob:" + globPattern);
        try (Stream<Path> stream = Files.find(root, Integer.MAX_VALUE,
                (path, attr) -> {
                    try {
                        Path rel = root.relativize(path);
                        return matcher.matches(rel);
                    } catch (IllegalArgumentException e) {
                        return false;
                    }
                })) {
            return (int) stream.count();
        }
    }
}
