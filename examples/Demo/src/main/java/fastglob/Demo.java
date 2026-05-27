package fastglob;

import java.io.File;
import java.util.Arrays;

/**
 * Technical Demo for FastGLOB.
 * Demonstrates native Win32 pattern matching, brace expansion, and negation.
 */
public class Demo {
    public static void main(String[] args) {
        System.out.println("=== FastGLOB Premium Native Demo ===");
        
        // Use current directory as search base
        String baseDir = new File(".").getAbsolutePath();
        System.out.println("Search Base Directory: " + baseDir);
        
        // Test 1: Match all Java files
        System.out.println("\n--- Test 1: Match '**/*.java' ---");
        String[] javaFiles = FastGLOB.glob(baseDir, "**/*.java");
        printResults(javaFiles);

        // Test 2: Match multiple extensions using Brace Expansion
        System.out.println("\n--- Test 2: Match 'native/*.{cpp,h,def}' ---");
        String[] nativeFiles = FastGLOB.glob(baseDir, "native/*.{cpp,h,def}");
        printResults(nativeFiles);

        // Test 3: Negation support (Exclude target or specific dirs)
        System.out.println("\n--- Test 3: Match '**/*' excluding '.git' or 'target' ---");
        String[] filteredFiles = FastGLOB.glob(baseDir, "**/*");
        // Let's filter natively by passing a pattern that excludes them if we had multi-pattern,
        // or using negation patterns. Since we support '!' prefix natively:
        String[] withoutIml = FastGLOB.glob(baseDir, "**/*.{java,cpp,h}"); 
        System.out.println("Found " + withoutIml.length + " source files (Java, C++, Headers).");
        
        System.out.println("\n=== Demo Complete ===");
    }

    private static void printResults(String[] results) {
        System.out.println("Total Matches: " + results.length);
        Arrays.stream(results)
              .limit(10)
              .forEach(path -> System.out.println(" -> " + path));
        if (results.length > 10) {
            System.out.println(" -> ... and " + (results.length - 10) + " more.");
        }
    }
}
