#ifndef FILETRAFFICFILTERPROFILE_H_INCLUDED
#define FILETRAFFICFILTERPROFILE_H_INCLUDED


#include "./cfiletrafficfilterprofile.hpp"

namespace dcw {

class FileTrafficFilterProfile : public CFileTrafficFilterProfile {
public:
  FileTrafficFilterProfile(const char * const name, const char * const filename);
  ~FileTrafficFilterProfile() final;
  FileTrafficFilterProfile(const FileTrafficFilterProfile& rhv); //no reason this cant be copied

  const char *GetFilename() const;
  FILE *fopen() const final;

private:
  const std::string _filename;
};

} // namespace dcw


#endif // #ifndef FILETRAFFICFILTERPROFILE_H_INCLUDED
