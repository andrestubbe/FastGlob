# FastGLOB — Native Windows Glob Pattern Matching for Java

**High-performance native Windows file pattern matching (globbing) and directory traversal library for Java.**

[![Build](https://img.shields.io/github/actions/workflow/status/andrestubbe/FastGlob/maven.yml?branch=main)](https://github.com/andrestubbe/FastGlob/actions)
[![Java](https://img.shields.io/badge/Java-17+-blue.svg)](https://www.java.com)
[![Platform](https://img.shields.io/badge/Platform-Windows%2010+-lightgrey.svg)]()
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![JitPack](https://jitpack.io/v/andrestubbe/FastGlob.svg)](https://jitpack.io/#andrestubbe/FastGlob)

**FastGLOB** is the high-performance globbing substrate of the FastJava ecosystem. It provides the hand-tuned Win32 native primitives required for lightning-fast directory traversal and pattern matching.

```java
// Quick Start — Example
import fastglob.FastGLOB;

public class Demo {
    public static void main(String[] args) {
        // Native Win32 glob search for all Java files in the current directory
        String[] files = FastGLOB.glob(".", "**/*.java");
        for (String file : files) {
            System.out.println("Found: " + file);
        }
    }
}
```

[![FastGLOB Showcase](docs/screenshot.png)](https://www.youtube.com/watch?v=HpCVFcFTWpg)

---

## Table of Contents
- [Key Features](#key-features)
- [Performance](#performance)
- [API Quick Reference](#api-quick-reference)
- [Installation](#installation)
- [Technical Examples & Hero Demos](#technical-examples--hero-demos)
- [Documentation](#documentation)
- [Platform Support](#platform-support)
- [License](#license)

---

## Why FastGLOB?

Java brings native globbing support via `java.nio.file.PathMatcher`, but it suffers from severe limitations in real-world, high-scale applications:

1. **🚨 Severe GC Pressure:** Traversing millions of files via `Files.walk` or `Files.find` instantiates millions of temporary `Path`, `File`, and `String` objects, triggering heavy Garbage Collection pauses. FastGLOB executes traversal and matching entirely in native C++, returning only the final, filtered matches to the JVM.
2. **🐌 Regular Expression Overhead:** Java's glob matcher compiles patterns into standard Java `Pattern` regex objects, which are slow and CPU-heavy when matched against millions of strings. FastGLOB uses hyper-optimized C++ wildcard algorithms.
3. **🚫 Semantic Shortcomings:** Standard Java globbing is notorious for unintuitive behavior (e.g., `**/*.xml` failing to match files in the root directory) and lacks advanced, modern matching features.

### ✨ Semantic Superiority
FastGLOB is not just faster; it is semantically superior and developer-friendly:
- **Gitignore-Compatible Syntax:** Matches patterns exactly as developers expect from `.gitignore`.
- **Brace Expansion:** Full support for alternative groupings (e.g., `*.{java,cpp,h}`).
- **Negation Support:** Exclude paths directly in the pattern (e.g., `!**/target/**`).
- **Consistent Double-Star (`**`):** Flawless recursive matching, including root-level files.

---

## Key Features
- **🚀 Native Performance** — Direct Win32 API access using native thread pools and lightweight directory queries (`NtQueryDirectoryFile`).
- **⚡ Zero GC Bloat** — Traversal, filtering, and glob matching happen entirely in C++; only matching results enter the JVM heap.
- **🛠️ Advanced Semantics** — Brace expansion, negation, and full `.gitignore` syntax compatibility.
- **📦 Zero Dependencies** — Just requires Java 17+ and Windows.

---

## 📊 Performance
FastGLOB is significantly faster than standard Java Alternatives, especially on large directory structures (such as projects with deep nested packages and Git directories). Below is the average performance measured over traversing and filtering the complete FastJava workspace directory tree (476 matching files):

| Environment | Standard Java NIO | FastGLOB Native | Speedup | Memory Garbage |
|-------------|-------------------|-----------------|---------|----------------|
| **Deep Workspace Search (`**/*.java`)** | 610 ms | **46 ms** | **13.2x faster! 🚀** | **0 MB** (Zero Heap Allocation) |

---

## API Quick Reference

| Method | Description | Path |
|--------|-------------|------|
| `static String[] glob(String baseDir, String pattern)` | Traverses the base directory and returns matched file paths relative to it. Supports brace expansion, negation patterns, and native directory pruning. | [Reference →](REFERENCE.md#glob) |

> [!TIP]
> See **[philosophie.md](philosophie.md)** for our Native-First architectural standards and JNI guidelines.

---

## 📥 Installation

FastJava modules are available via JitPack. Depending on the module type (Pure-Java or JNI-Native), select the appropriate integration:

*   **Pure-Java Modules:** Only require the main module dependency.
*   **JNI-Native Modules:** Require **two** dependencies: the module itself and `FastCore` (the mandatory native DLL loader).

### Option 1: Maven (JitPack)
Add the JitPack repository and the dependencies to your `pom.xml`:
```xml
<repositories>
    <repository>
        <id>jitpack.io</id>
        <url>https://jitpack.io</url>
    </repository>
</repositories>

<dependencies>
    <!-- 1. The main Module -->
    <dependency>
        <groupId>com.github.andrestubbe</groupId>
        <artifactId>FastGlob</artifactId>
        <version>v0.1.0</version>
    </dependency>
    
    <!-- 2. FastCore (Required ONLY for JNI-Native Modules) -->
    <dependency>
        <groupId>com.github.andrestubbe</groupId>
        <artifactId>fastcore</artifactId>
        <version>v1.0.0</version>
    </dependency>
</dependencies>
```

### Option 2: Gradle (JitPack)
Add this to your `build.gradle` file:
```gradle
repositories {
    maven { url 'https://jitpack.io' }
}

dependencies {
    implementation 'com.github.andrestubbe:FastGlob:v0.1.0'
    implementation 'com.github.andrestubbe:fastcore:v1.0.0' // Required ONLY for JNI-Native Modules
}
```

### Option 3: Direct Download (No Build Tool)
Download the latest pre-compiled JARs directly to add them to your project's classpath:

1. 📦 [**FastGlob-v0.1.0.jar**](https://github.com/andrestubbe/FastGLOB/releases) (The Core Library)
2. ⚙️ [**fastcore-v1.0.0.jar**](https://github.com/andrestubbe/FastCore/releases) (The Mandatory JNI Loader — ONLY for JNI-Native Modules)

> [!IMPORTANT]
> Both JARs must be present in your classpath for FastGLOB's native functions to operate correctly.

---

## Technical Examples & Hero Demos
See the `examples/` directory for technical implementations and performance races:

| Case | Java Example / Demo | JMH Benchmark / Race |
|------|---------------------|----------------------|
| **Pattern Matcher** | [Demo.java](examples/Demo/src/main/java/fastglob/Demo.java) | — |
| **Directory Search Performance** | — | [Benchmark.java](examples/Benchmark/src/main/java/fastglob/Benchmark.java) |

---

## Documentation
*   **[REFERENCE.md](REFERENCE.md)**: Full technical specification and JNI contracts.
*   **[PHILOSOPHIE.md](PHILOSOPHIE.md)**: The "Native-First" philosophy.
*   **[CHANGELOG.md](CHANGELOG.md)**: Project history.
*   **[ROADMAP.md](ROADMAP.md)**: Future development and milestones.

---

## Platform Support
| Platform | Status |
|----------|--------|
| Windows 10/11 (x64) | ✅ Fully Supported |
| Linux | 🚧 Planned |
| macOS | 🚧 Planned |

---

## License
MIT License — See [LICENSE](LICENSE) file for details.

---

## Related Projects

- [FastFileIndex](https://github.com/andrestubbe/FastFileIndex) — Ultra-fast filesystem scanner
- [FastFileContentIndex](https://github.com/andrestubbe/FastFileContentIndex) — High-speed in-file text indexing
- [FastFileWatch](https://github.com/andrestubbe/FastFileWatch) — High-performance directory watch service using USN Journal
- [FastFileSearch](https://github.com/andrestubbe/FastFileSearch) — Ultra-fast indexed file prefix trie search
- [FastFileScrape](https://github.com/andrestubbe/FastFileScrape) — High-performance native file scraping and chunking
- [FastFileSystem](https://github.com/andrestubbe/FastFileSystem) — Unified filesystem operations (Index, Search, Watch, Scrape) in one API

---

**Part of the FastJava Ecosystem** — *Making the JVM faster. Small package. Maximum speed. Zero bloat. 🚀📋*

