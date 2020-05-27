#include <os/File.h>
#include <os/Path.h>
#include <stdio.h>
#include <string>

#define dotest(condition)                                                      \
  if (!(condition)) {                                                          \
    fprintf(stderr, "FAIL(%s:%d): %s\n", __FILE__, __LINE__, #condition);      \
  }

int main(int, const char *const[]) {
  int iterations = 200;
#ifdef __Tracer_h__
  iterations = 1;
#endif
  for (int i = 0; i < iterations; ++i) {
    try {
      io::Path working("bin/logs//");
      io::Path test(working + "Path_test_file.txt");
      io::Path::StringList listing;

      dotest(test.extension() == "txt");
      dotest(test.basename() == "Path_test_file");
      dotest(working.extension() == "");
      dotest(working.basename() == "logs");

      if (io::Path("test").name() != "test") {
        printf("FAILED: 'test' name was %s\n", io::Path("test").name().c_str());
      }
      if (!test.parent().isDirectory()) {
        printf("FAILED: Parent directory does not exist: %s\n",
               std::string(test).c_str());
      }
      if (test.isAbsolute()) {
        printf("FAILED: Should not be absolute: %s\n",
               std::string(test).c_str());
      }
      if (test.isEmpty()) {
        printf("FAILED: Should not be empty: %s\n", std::string(test).c_str());
      }
      if (!test.isRelative()) {
        printf("FAILED: Should be relative: %s\n", std::string(test).c_str());
      }

      if (test.isFile()) {
        test.remove();
      }
      { io::File(test, io::File::Text, io::File::ReadWrite).write("test"); }
      if (!test.isFile()) {
        printf("FAILED: Created %s but it isn't a file\n",
               std::string(test).c_str());
      }

      if ((working + "Path_test_file_2.txt").isFile()) {
        (working + "Path_test_file_2.txt").remove();
      }

      test.copyContentsTo(working + "Path_test_file_2.txt");

      if ((working + "Path_test_file_2.txt").contents() != "test") {
        printf("FAILED: File copy has different contents\n");
      }

      if (test.isDirectory()) {
        printf("FAILED: Created %s as a file but it is a directory\n",
               std::string(test).c_str());
      }
      if (test.isLink()) {
        printf("FAILED: Created %s as a file but it is a link\n",
               std::string(test).c_str());
      }
      test.remove();
      if (test.isFile()) {
        printf("FAILED: Deleted %s but the file is still there\n",
               std::string(test).c_str());
      }
      if (test.exists()) {
        printf("FAILED: Deleted %s but it is still there\n",
               std::string(test).c_str());
      }
      if (test.isLink()) {
        printf("FAILED: Deleted %s but a link is there\n",
               std::string(test).c_str());
      }

      test = working + "Path_test_directory";
      if (test.isDirectory()) {
        test.remove();
      }
      test.mkdir();
      if (test.isFile()) {
        printf("FAILED: Created %s as a directory but it is a file\n",
               std::string(test).c_str());
      }
      if (!test.isDirectory()) {
        printf("FAILED: Created %s but it is not a directory\n",
               std::string(test).c_str());
      }
      if (test.isLink()) {
        printf("FAILED: Created %s as a directory but it is a link\n",
               std::string(test).c_str());
      }
      test.remove();
      if (test.isDirectory()) {
        printf("FAILED: Deleted %s but the directory is still there\n",
               std::string(test).c_str());
      }
      if (test.exists()) {
        printf("FAILED: Deleted %s but it is still there\n",
               std::string(test).c_str());
      }
      if (test.isLink()) {
        printf("FAILED: Deleted %s but a link is there\n",
               std::string(test).c_str());
      }
      if (!io::Path("test").parent().isEmpty()) {
        printf("FAILED: 'test' parent is not empty\n");
      }
      if (std::string(io::Path("/test").parent()) != "/") {
        printf("FAILED: '/test' parent is not '/'\n");
      }
      listing = working.parent().list(io::Path::PathAndName);
      const auto flatListingSize = listing.size();
      bool found = false;
      printf("Listing of: %s\n", ((io::Path::String)working.parent()).c_str());
      for (auto item = listing.begin(); !found && (item != listing.end());
           ++item) {
        io::Path entry(*item);

        printf(
            "%16s inode=%ld:%llu permissions=%o links=%ld uid=%d gid=%d "
            "size=%llu size on disk=%llu created=%s modified=%s updated=%s "
            "accessed=%s\n",
            entry.name().c_str(), static_cast<unsigned long>(entry.device()),
            static_cast<unsigned long long>(entry.inode()), entry.permissions(),
            static_cast<unsigned long>(entry.links()), entry.userId(),
            entry.groupId(), static_cast<unsigned long long>(entry.size()),
            static_cast<unsigned long long>(entry.blocks() * entry.blockSize()),
            entry.created().format("%Y/%m/%d %H:%M:%S").c_str(),
            entry.lastModification().format("%Y/%m/%d %H:%M:%S").c_str(),
            entry.lastStatusChange().format("%Y/%m/%d %H:%M:%S").c_str(),
            entry.lastAccess().format("%Y/%m/%d %H:%M:%S").c_str());
        found = found || (entry.name() == "test");
      }
      if (!found) {
        printf("FAILED: could not find 'test' in %s\n",
               std::string(working.parent()).c_str());
      }

      listing = working.parent().list(io::Path::PathAndName,
                                      io::Path::RecursiveListing);
      if (listing.size() <= flatListingSize) {
        printf("FAILED: Recursive should have more entries than flat listing "
               "'%s' flat=%lu recursive=%lu\n",
               std::string(test).c_str(), flatListingSize, listing.size());
      }

      try {
        test.list(io::Path::NameOnly);
        printf("FAILED: we cannot list '%s'\n", std::string(test).c_str());
      } catch (const posix::err::ENOENT_Errno &) {
      }
      (working + "dir1" + "dir2" + "dir3").mkdirs();
      (working + "dir1").remove();
      if ((working + "dir1").isDirectory()) {
        printf("FAILED: We tried to delete %s, but its still there.\n",
               std::string(working + "dir1").c_str());
      }

      dotest(io::Path("/alpha/beta/gamma/")
                 .relativeTo("/alpha/beta/gamma")
                 .isEmpty());
      dotest(io::Path("/alpha/beta/gamma")
                 .relativeTo("/alpha/beta/gamma")
                 .isEmpty());
      dotest(io::Path("/alpha/beta/gamma/")
                 .relativeTo("/alpha/beta/gamma/")
                 .isEmpty());
      dotest(io::Path("/alpha/beta/gamma")
                 .relativeTo("/alpha/beta/gamma/")
                 .isEmpty());

      printf(
          "io::Path(\"/alpha/b\").relativeTo(\"/alpha/beta/gamma\") = '%s'\n",
          std::string(io::Path("/alpha/beta").relativeTo("/alpha/beta/gamma"))
              .c_str());

      dotest(io::Path("/alpha/beta").relativeTo("/alpha/beta/gamma") ==
             io::Path(".."));
      dotest(io::Path("/alpha/beta/").relativeTo("/alpha/beta/gamma") ==
             io::Path(".."));
      dotest(io::Path("/alpha/beta").relativeTo("/alpha/beta/gamma/") ==
             io::Path(".."));
      dotest(io::Path("/alpha/beta/").relativeTo("/alpha/beta/gamma/") ==
             io::Path(".."));

      printf(
          "io::Path(\"/alpha/beta/gamma\").relativeTo(\"/alpha/beta\") = "
          "'%s'\n",
          std::string(io::Path("/alpha/beta/gamma").relativeTo("/alpha/beta"))
              .c_str());

      dotest(io::Path("/alpha/beta/gamma").relativeTo("/alpha/beta") ==
             io::Path("gamma"));
      dotest(io::Path("/alpha/beta/gamma/").relativeTo("/alpha/beta") ==
             io::Path("gamma"));
      dotest(io::Path("/alpha/beta/gamma").relativeTo("/alpha/beta/") ==
             io::Path("gamma"));
      dotest(io::Path("/alpha/beta/gamma/").relativeTo("/alpha/beta/") ==
             io::Path("gamma"));

      dotest(io::Path("/alpha/beta/gamma/delta/epsilon/theta")
                 .relativeTo("/alpha/beta/gamma/phi/zeta/pi/omicron") ==
             io::Path("../../../../delta/epsilon/theta"));
      dotest(io::Path("/alpha/beta/gamma/delta/epsilon/theta/")
                 .relativeTo("/alpha/beta/gamma/phi/zeta/pi/omicron") ==
             io::Path("../../../../delta/epsilon/theta"));
      dotest(io::Path("/alpha/beta/gamma/delta/epsilon/theta")
                 .relativeTo("/alpha/beta/gamma/phi/zeta/pi/omicron/") ==
             io::Path("../../../../delta/epsilon/theta"));
      dotest(io::Path("/alpha/beta/gamma/delta/epsilon/theta/")
                 .relativeTo("/alpha/beta/gamma/phi/zeta/pi/omicron/") ==
             io::Path("../../../../delta/epsilon/theta"));

      dotest(!io::Path("/tmp").uniqueName().exists());

      try {
        io::Path("test").relativeTo("/other");
        dotest(false);
      } catch (const msg::Exception &exception) {
        printf("Expected exception: %s\n", exception.what());
      }

      try {
        io::Path("/test").relativeTo("other");
        dotest(false);
      } catch (const msg::Exception &exception) {
        printf("Expected exception: %s\n", exception.what());
      }
    } catch (const std::exception &exception) {
      printf("FAILED: Exception: %s\n", exception.what());
    }
  }
  return 0;
}
