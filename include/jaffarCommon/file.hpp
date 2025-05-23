#pragma once

/**
 * @file file.hpp
 * @brief Contains common functions related to file manipulation
 */

#include <cstdio>
#include <cstring>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <string>

namespace jaffarCommon
{

namespace file
{

/**
 * Function to read an entire input file file into a standard string
 *
 * Taken from https://stackoverflow.com/questions/116038/how-do-i-read-an-entire-file-into-a-stdstring-in-c/116220#116220
 *
 * @param[in] in The input file file
 * @return The produced string containing the entire input from the file
 */
static __JAFFAR_COMMON_INLINE__ std::string slurp(std::ifstream& in)
{
  std::ostringstream sstr;
  sstr << in.rdbuf();
  return sstr.str();
}

/**
 * Reads an entire file into a string
 *
 * @param[out] dst The output string onto which to save the read file
 * @param[in] fileName The name of the file to read
 * @return Whether the load operation succeded (true) or failed (fail)
 */
static __JAFFAR_COMMON_INLINE__ bool loadStringFromFile(std::string& dst, const std::string& fileName)
{
  std::ifstream fi(fileName);

  // If file not found or open, return false
  if (fi.good() == false) return false;

  // Reading entire file
  dst = slurp(fi);

  // Closing file
  fi.close();

  return true;
}

/**
 * Write a string into a file
 *
 * @param[in] src The source string to save into the file
 * @param[in] fileName The name of the file to write to
 * @return Whether the save operation succeded (true) or failed (fail)
 */
static __JAFFAR_COMMON_INLINE__ bool saveStringToFile(const std::string& src, const std::string& fileName)
{
  FILE* fid = fopen(fileName.c_str(), "w");
  if (fid != NULL)
  {
    fwrite(src.c_str(), 1, src.size(), fid);
    fclose(fid);
    return true;
  }
  return false;
}

class MemoryFileDirectory;

/**
 * Represents a file object that exists entirely in memory.
 * It exposes a C-like interface for light refactoring of programs that depend on accessing the filesystem directly
 */
class MemoryFile
{
public:
  friend class MemoryFileDirectory;

  MemoryFile()                      = default;
  MemoryFile(const MemoryFile&)     = delete;
  void operator=(const MemoryFile&) = delete;

  /**
   * Destructor for the memory file class. it frees the buffer created at allocation time
   */
  ~MemoryFile() { free(_buffer); }

  /**
   * Reads from the mem file into a buffer.
   *
   * Reading advances the internal head for as many bytes as size*count, until the end of file.
   *
   * @param[in] buffer The buffer onto which to copy the data
   * @param[in] size The size of each element to copy
   * @param[in] count The number of elements to copy
   * @param[in] file The memory file from which to read
   *
   * @return The number of bytes read. Negative in case of error.
   */
  static __JAFFAR_COMMON_INLINE__ int64_t fread(void* const buffer, const size_t size, const size_t count, MemoryFile* const file)
  {
    // Check if file is closed
    if (file->isOpened() == false)
    {
      file->_errorCode = -1;
      return file->_errorCode;
    }

    // Refuse operation if file is write only
    if (file->isWriteOnly() == true)
    {
      file->_errorCode = -2;
      return file->_errorCode;
    }

    // Ensuring we don't exceed mem buffer size
    size_t newCount = count;
    if (file->_head + (size * count) > file->_size) newCount = (file->_size - file->_head) / size;

    // Getting requested size
    const size_t requestedSize = size * newCount;

    // Performing memcpy
    if (requestedSize > 0) std::memcpy(buffer, &file->_buffer[file->_head], requestedSize);

    // Advancing head
    file->_head += requestedSize;

    // Calling corresponding callback, if defined
    if (file->_readCallbackDefined == true) file->_readCallback(requestedSize, file);

    // Returning element count read
    file->_errorCode = 0;
    return newCount;
  }

  /**
   * Writes into the mem file from a buffer
   *
   * Writing advances the internal head for as many bytes as size*count, until the end of file.
   *
   * @param[in] buffer The buffer from which to read the data
   * @param[in] size The size of each element to copy
   * @param[in] count The number of elements to copy
   * @param[in] file The memory file to write into
   *
   * @return The number of bytes written. Negative in case of error.
   */
  static __JAFFAR_COMMON_INLINE__ int64_t fwrite(const void* buffer, const size_t size, const size_t count, MemoryFile* const file)
  {
    // Check if file is closed
    if (file->isOpened() == false)
    {
      file->_errorCode = -1;
      return file->_errorCode;
    }

    // Refuse operation if file is read only
    if (file->isReadOnly() == true)
    {
      file->_errorCode = -2;
      return file->_errorCode;
    }

    // Getting requested size
    const size_t requestedSize = size * count;

    // Checking if internal buffer needs to be resized
    const size_t endHeadPos = file->_head + requestedSize;
    if (endHeadPos > file->_bufferSize) file->resizeToFit(endHeadPos + 1);

    // Performing memcpy
    if (requestedSize > 0) memcpy(&file->_buffer[file->_head], buffer, requestedSize);

    // Advancing head until the next
    file->_head += requestedSize;
    if (file->_head > file->_size) file->_size = file->_head;

    // Calling corresponding callback, if defined
    if (file->_writeCallbackDefined == true) file->_writeCallback(requestedSize, file);

    // Returning element count written
    file->_errorCode = 0;
    return count;
  }

  /**
   * (mirror for ftell - has no different effect or interface)
   * Returns the internal position of the file's head
   *
   * @param[in] file The file to evaluate
   * @return The internal position of the head. -1 if the file is not open
   */
  static __JAFFAR_COMMON_INLINE__ int64_t ftello64(MemoryFile* const file) { return ftell(file); }

  /**
   * Returns the internal position of the file's head
   *
   * @param[in] file The file to evaluate
   * @return The internal position of the head. -1 if the file is not open
   */
  static __JAFFAR_COMMON_INLINE__ int64_t ftell(MemoryFile* const file)
  {
    // Check if file is closed
    if (file->isOpened() == false)
    {
      file->_errorCode = -1;
      return file->_errorCode;
    }

    file->_errorCode = 0;
    return file->_head;
  }

  /**
   * Resets the internal position of the file's head to the start of the file
   *
   * @param[in] file The file to rewind
   */
  static __JAFFAR_COMMON_INLINE__ void rewind(MemoryFile* const file)
  {
    // Check if file is closed
    if (file->isOpened() == false)
    {
      file->_errorCode = -1;
      return;
    }

    file->_errorCode = 0;
    file->_head      = 0;
  }

  /**
   * Ensures the write operations have finished. No effect for mem buffers as all operations finish within their call.
   *
   * @param[in] file The file to flush
   * @return Zero in case of success. -1 in case of error
   */
  static __JAFFAR_COMMON_INLINE__ int fflush(MemoryFile* file)
  {
    // Check if file is closed
    if (file->isOpened() == false)
    {
      file->_errorCode = -1;
      return file->_errorCode;
    }

    file->_errorCode = 0;
    return 0;
  }

  /**
   * (mirror for fseek - has no different effect or interface)
   * Ensures the write operations have finished. No effect for mem buffers as all operations finish within their call.
   *
   * @param[in] file The file whose head to move
   * @param[in] offset The number of bytes to move the internal head
   * @param[in] origin The point relative to which we apply the offset
   *
   * @return Zero in case of success. -1 in case of error
   */
  static __JAFFAR_COMMON_INLINE__ int fseeko64(MemoryFile* const file, const int64_t offset, const int origin) { return fseek(file, offset, origin); }

  /**
   * Ensures the write operations have finished. No effect for mem buffers as all operations finish within their call.
   *
   * @param[in] file The file whose head to move
   * @param[in] offset The number of bytes to move the internal head
   * @param[in] origin The point relative to which we apply the offset
   *
   * @return Zero in case of success. -1 in case of error
   */
  static __JAFFAR_COMMON_INLINE__ int fseek(MemoryFile* const file, const int64_t offset, const int origin)
  {
    // Check if file is closed
    if (file->isOpened() == false)
    {
      file->_errorCode = -1;
      return file->_errorCode;
    }

    int64_t startPos = file->_head;
    if (origin == SEEK_SET) startPos = 0;
    if (origin == SEEK_END) startPos = file->_size;

    int64_t desiredPos = startPos + offset;
    if (desiredPos < 0)
    {
      file->_errorCode = -2;
      return file->_errorCode;
    }
    if (desiredPos > (int64_t)file->_size)
    {
      file->_errorCode = -3;
      return file->_errorCode;
    }

    file->_head      = desiredPos;
    file->_errorCode = 0;
    return 0;
  }

  /**
   * Indicates whether we've reached the end of file
   *
   * @param[in] file The file to inquire for end of file
   * @return Non-zero, if the end has been reached. Zero, otherwise.
   */
  static __JAFFAR_COMMON_INLINE__ int feof(MemoryFile* const file)
  {
    // Check if file is closed
    if (file->isOpened() == false)
    {
      file->_errorCode = -1;
      return file->_errorCode;
    }

    file->_errorCode = 0;
    return file->_head == file->_size;
  }

  /**
   * Clears the error code(s)
   *
   * @param[in] file The file to clear errors for
   */
  static __JAFFAR_COMMON_INLINE__ void clearerr(MemoryFile* const file)
  {
    // Check if file is closed
    if (file->isOpened() == false) return;

    file->_errorCode = 0;
  }

  /**
   * Returns the value of the internal error value corresponding to the last operation
   *
   * @param[in] file The file to check errors for
   * @return Zero, if last operation was successful. Non-zero, otherwise.
   */
  static __JAFFAR_COMMON_INLINE__ int ferror(MemoryFile* const file) { return file->_errorCode; }

  /**
   * Sets the read only flag in the file
   */
  __JAFFAR_COMMON_INLINE__ void setReadOnly() { _readonly = true; }

  /**
   * Clears the read only flag from the file
   */
  __JAFFAR_COMMON_INLINE__ void unsetReadOnly() { _readonly = false; }

  /**
   * Sets the write only flag in the file
   */
  __JAFFAR_COMMON_INLINE__ void setWriteOnly() { _writeonly = true; }

  /**
   * Clears the read only flag from the file
   */
  __JAFFAR_COMMON_INLINE__ void unsetWriteOnly() { _writeonly = false; }

  /**
   * Sets the opened flag in the file
   */
  __JAFFAR_COMMON_INLINE__ void setOpened() { _opened = true; }

  /**
   * Clears the opened flag from the file
   */
  __JAFFAR_COMMON_INLINE__ void unsetOpened() { _opened = false; }

  /**
   * Gets the read-only flag from the file
   * @return The file's read-only flag
   */
  __JAFFAR_COMMON_INLINE__ bool isReadOnly() const { return _readonly; }

  /**
   * Gets the write-only flag from the file
   * @return The file's write-only flag
   */
  __JAFFAR_COMMON_INLINE__ bool isWriteOnly() const { return _writeonly; }

  /**
   * Gets the opened flag from the file
   * @return The file's opened flag
   */
  __JAFFAR_COMMON_INLINE__ bool isOpened() const { return _opened; }

  /**
   * Sets a callback that will be called when a write is made
   *
   * @param[in] callback Write-callback to set
   */
  __JAFFAR_COMMON_INLINE__ void setWriteCallback(const std::function<void(const int64_t, MemoryFile*)> callback)
  {
    _writeCallback        = callback;
    _writeCallbackDefined = true;
  }

  /**
   * Sets a callback that will be called when a read is made
   *
   * @param[in] callback Read-callback to set
   */
  __JAFFAR_COMMON_INLINE__ void setReadCallback(const std::function<void(const int64_t, MemoryFile*)> callback)
  {
    _readCallback        = callback;
    _readCallbackDefined = true;
  }

  /**
   * Function to unset the write callback
   */
  __JAFFAR_COMMON_INLINE__ void unsetWriteCallback() { _writeCallbackDefined = false; }

  /**
   * Function to unset the read callback
   */
  __JAFFAR_COMMON_INLINE__ void unsetReadCallback() { _readCallbackDefined = false; }

  /**
   * Function to resize file
   * It fills with zeroes any additional bytes allocated
   * In case of shrinking, this operation keeps the internal buffer unchanged (no freed space).
   *
   * @param[in] newSize The new desired size for the file
   * @return Zero, if successful. Non-zero if an error ocurred.
   */
  __JAFFAR_COMMON_INLINE__ int resize(const size_t newSize)
  {
    // Check if file is closed
    if (isOpened() == false)
    {
      _errorCode = -1;
      return _errorCode;
    }

    // Refuse operation if file is read only
    if (isReadOnly() == true)
    {
      _errorCode = -2;
      return _errorCode;
    }

    // First, assign new size
    // const size_t oldSize = _size;
    _size = newSize;

    // Then, resize the internal buffer, if needed
    if (_bufferSize < _size)
    {
      resizeToFit(_size);
      if (_buffer == nullptr) return -1;
    }

    // Then check head in case of shrinking file
    if (_head > _size) _head = _size;

    _errorCode = 0;
    return 0;
  }

  /**
   * Function to set size directly without resizing internal buffer -- use at your own risk.
   *
   * @param[in] size The internal size to set
   */
  __JAFFAR_COMMON_INLINE__ void setSize(const size_t size) { _size = size; }

  /**
   * Function to get file size directly
   *
   * @return The internal size to set
   */
  __JAFFAR_COMMON_INLINE__ size_t getSize() const { return _size; }

private:
  uint8_t* getBuffer() const { return _buffer; }

  void resizeToFit(const size_t target)
  {
    // Getting current buffer size
    size_t newBufferSize = _bufferSize;
    if (newBufferSize == 0) newBufferSize = 1;

    // Duplicating new buffer size until the target fits
    while (newBufferSize < target) newBufferSize <<= 1;

    // Reallocating buffer
    _bufferSize = newBufferSize;
    _buffer     = (uint8_t*)realloc(_buffer, newBufferSize);
  }

  /**
   * The file's logical size
   * Starts at zero if the file is new -- grows with write operations
   */
  size_t _size = 0;

  /**
   * The file's internal buffer size (increases on demand)
   */
  size_t _bufferSize = 0;

  /**
   * The file's buffer
   */
  uint8_t* _buffer = nullptr;

  /**
   * The file's read-only flag
   */
  bool _readonly = false;

  /**
   * The file's write-only flag
   */
  bool _writeonly = false;

  /**
   * The file's opened flag
   */
  bool _opened = false;

  /**
   * The file's internal head pointer
   */
  size_t _head = 0;

  /**
   * Storage for an error code
   */
  int _errorCode = 0;

  /**
   * Whether the write callback has been defined
   */
  bool _writeCallbackDefined = false;

  /**
   * The file's internal callback for writes
   */
  std::function<void(const int64_t, MemoryFile* const)> _writeCallback;

  /**
   * Whether the write callback has been defined
   */
  bool _readCallbackDefined = false;

  /**
   * The file's internal callback for read
   */
  std::function<void(const int64_t, MemoryFile* const)> _readCallback;
};

/**
 * This class defines a directory on which files can be created, opened, closed and re-opened later.
 * The file's lifetime is that of the directory itself, unless purposefully destroyed.
 */
class MemoryFileDirectory
{
public:
  MemoryFileDirectory() = default;

  /**
   * Opens a file
   * The behaviour imitates that of the POSIX fopen, including mode. However, it also adds the size argument for the creation of new files.
   * Appending mode is not supported as this assumes the buffer will increase, which is not currently possible for mem buffers
   *
   * @param[in] filename The file of the name to open
   * @param[in] mode The opening mode (r,w,a,+)
   *
   * @return The pointer to the new memory file, if successful. NULL, otherwise.
   */
  __JAFFAR_COMMON_INLINE__ MemoryFile* fopen(const std::string filename, const std::string mode)
  {
    // Parsing mode
    mode_t openMode = mode_t::none;
    if (mode.find("r") != std::string::npos) openMode = mode_t::read;
    if (mode.find("w") != std::string::npos)
    {
      if (openMode != mode_t::none) return NULL; // Conflicting modes selected
      openMode = mode_t::write;
    }
    if (mode.find("a") != std::string::npos)
    {
      if (openMode != mode_t::none) return NULL; // Conflicting modes selected
      openMode = mode_t::append;
    }

    // If appending, fail because mem buffers have static size
    if (openMode == mode_t::append) return NULL;

    // If no mode chosen, fail
    if (openMode == mode_t::none) return NULL;

    // Parsing extended
    bool extendedMode = false;
    if (mode.find("+") != std::string::npos) extendedMode = true;

    // Parsing extended
    bool noCreateMode = false;
    if (mode.find("x") != std::string::npos) noCreateMode = true;

    // Checking if file already exists
    bool fileExists = _fileMap.contains(filename);

    // Check if file needs to be created
    bool createFile = false;

    // Evaluating the case where the file doesn't exist
    if (fileExists == false)
    {
      // Check if the no-create flag has been provided
      if (noCreateMode == true) return NULL;

      // If reading, return NULL
      if (openMode == mode_t::read) return NULL;

      // Otherwise, create a new one
      createFile = true;
    }

    // Evaluating the case where the file does exist
    if (fileExists == true)
    {
      // Check if opened. If it is, then we cannot re-open it now
      if (_fileMap.at(filename)->isOpened() == true) return NULL;

      // If writing, overwrite file
      if (openMode == mode_t::write) createFile = true;
    }

    // Creating new file, if required
    if (createFile == true) _fileMap[filename] = std::make_unique<MemoryFile>();

    // Getting file
    MemoryFile* file = _fileMap.at(filename).get();

    // Otherwise, we set it as opened
    file->setOpened();

    // If reading, set as read only
    if (openMode == mode_t::read)
    {
      file->setReadOnly();
      file->unsetWriteOnly();
    }

    // If writing, set as write only
    if (openMode == mode_t::write)
    {
      file->setWriteOnly();
      file->unsetReadOnly();
    }

    // If extended mode, all operations are permitted
    if (extendedMode == true)
    {
      file->unsetWriteOnly();
      file->unsetReadOnly();
    }

    // Set file head to the start
    MemoryFile::rewind(file);

    // Return file
    return file;
  }

  /**
   * Closes the provided file
   *
   * @param[in] file The file to close
   * @return Zero, if successful. Negative, if the file wasn't opened.
   */
  int fclose(MemoryFile* const file)
  {
    if (file == NULL) return -1;

    // Check if file already closed
    if (file->isOpened() == false) return -2;

    // Set the file as closed
    file->unsetOpened();

    return 0;
  }

  /**
   * Forcibly destroys a file by name (similar to rm)
   *
   * @param filename Name of the file to delete from the directory
   * @return Zero, if successful. Negative if error.
   */
  int fdestroy(const std::string& filename)
  {
    // Checking if file already exists
    if (_fileMap.contains(filename) == false) return -1;

    // Getting file
    MemoryFile* file = _fileMap.at(filename).get();

    // Check if opened. Cannot delete it yet
    if (file->isOpened() == true) return -2;

    // Delete it now
    _fileMap.erase(filename);

    return 0;
  }

  /**
   *  Checks if the directory contains the given file
   *
   * @param[in] filename The name of the file to check for
   * @return True, if the file exists. False, otherwise.
   */
  bool contains(const std::string& filename) const { return _fileMap.contains(filename); }

  /**
   * Retrieves a file's size
   *
   * @param[in] filename The name of the file to check size for
   * @return The file size, if file is found; -1, otherwise.
   */
  int64_t getFileSize(const std::string& filename) const
  {
    if (contains(filename) == false) return -1;
    return _fileMap.at(filename)->getSize();
  }

  /**
   * Retrieves a file's internal buffer
   *
   * @param[in] filename The name of the file to get the internal buffer from
   * @return A pointer to the file's internal buffer, if file is found; nullptr, otherwise.
   */
  uint8_t* getFileBuffer(const std::string& filename) const
  {
    if (contains(filename) == false) return nullptr;
    return _fileMap.at(filename)->getBuffer();
  }

private:
  /**
   * Modes in which the file can be opened
   */
  enum mode_t
  {
    none,
    read,
    write,
    append
  };

  /**
   * Internal file map
   */
  std::map<std::string, std::unique_ptr<MemoryFile>> _fileMap;
};

} // namespace file

} // namespace jaffarCommon
