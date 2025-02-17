#pragma once

/**
 * @file file.hpp
 * @brief Contains common functions related to file manipulation
 */

#include <fstream>
#include <sstream>
#include <string>
#include <cstdio>

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
static __INLINE__ std::string slurp(std::ifstream &in)
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
static __INLINE__ bool loadStringFromFile(std::string &dst, const std::string &fileName)
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
static __INLINE__ bool saveStringToFile(const std::string &src, const std::string &fileName)
{
  FILE *fid = fopen(fileName.c_str(), "w");
  if (fid != NULL)
    {
      fwrite(src.c_str(), 1, src.size(), fid);
      fclose(fid);
      return true;
  }
  return false;
}

/**
 * Represents a file object that exists entirely in memory.
 * It exposes a C-like interface for light refactoring of programs that depend on accessing the filesystem directly
 */
class MemoryFile
{
  public:

  MemoryFile() = delete;
  MemoryFile(const MemoryFile&) = delete;
  void operator=(const MemoryFile&) = delete;

  /**
   * Constructor for the memory file class. 
   * 
   * @param[in] size The size of the internal buffer for the file. This buffer is static and cannot be increased.
   */
  MemoryFile(const size_t size)
    : _size(size)
    , _buffer(_size == 0 ? nullptr : (uint8_t *)calloc(1, _size))
    , _head(0)
  {}

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
  static __INLINE__ ssize_t fread(void *const buffer, const size_t size, const size_t count, MemoryFile *const file)
  {
    // Check if file is closed
    if (file->isOpened() == false) return -1;

    // Refuse operation if file is write only
    if (file->isWriteOnly() == true) return -2;

    // Getting requested size
    const size_t requestedSize = size * count;

    // Ensuring we don't exceed mem buffer size
    if (file->_head + requestedSize > file->_size) return -3;

    // Performing memcpy
    if (requestedSize > 0) memcpy(buffer, file->_buffer, requestedSize);

    // Advancing head
    file->_head += requestedSize;

    // Calling corresponding callback, if defined
    if (file->_readCallbackDefined == true) file->_readCallback(requestedSize, file);

    // Returning effective size of bytes read
    return requestedSize;
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
  static __INLINE__ ssize_t fwrite(const void *buffer, const size_t size, const size_t count, MemoryFile *const file)
  {
    // Check if file is closed
    if (file->isOpened() == false) return -1;

    // Refuse operation if file is read only
    if (file->isReadOnly() == true) return -2;

    // Getting requested size
    const size_t requestedSize = size * count;

    // Ensuring we don't exceed mem buffer size
    if (file->_head + requestedSize > file->_size) return -3;

    // Performing memcpy
    if (requestedSize > 0) memcpy(file->_buffer, buffer, requestedSize);

    // Advancing head until the next 
    file->_head += requestedSize;

    // Calling corresponding callback, if defined
    if (file->_writeCallbackDefined == true) file->_writeCallback(requestedSize, file);

    // Returning effective number of bytes read
    return requestedSize;
  }

  /**
   * Returns the internal position of the file's head
   * 
   * @param[in] file The file to evaluate
   * @return The internal position of the head. -1 if the file is not open
   */
  static __INLINE__ ssize_t ftell(const MemoryFile *const file)
  {
    // Check if file is closed
    if (file->isOpened() == false) return -1;

    return file->_head;
  }

  /**
   * Resets the internal position of the file's head to the start of the file
   * 
   * @param[in] file The file to rewind
   */
  static __INLINE__ void rewind(MemoryFile *const file)
  {
    // Check if file is closed
    if (file->isOpened() == false) return;

    file->_head = 0;
  }

  /**
   * Ensures the write operations have finished. No effect for mem buffers as all operations finish within their call.
   * 
   * @param[in] file The file to flush
   * @return Zero in case of success. -1 in case of error
   */
  static __INLINE__ int fflush(MemoryFile *file)
  {
    // Check if file is closed
    if (file->isOpened() == false) return -1;

    return 0;
  }

  /**
   * Ensures the write operations have finished. No effect for mem buffers as all operations finish within their call.
   * 
   * @param[in] file The file whose head to move
   * @param[in] offset The number of bytes to move the internal head
   * @param[in] origin The point relative to which we apply the offset
   * 
   * @return Zero in case of success. -1 in case of error
   */
  static __INLINE__ int fseek(MemoryFile *const file, const ssize_t offset, const int origin)
  {
    // Check if file is closed
    if (file->isOpened() == false) return -1;

    ssize_t startPos = file->_head;
    if (origin == SEEK_SET) startPos = 0;
    if (origin == SEEK_END) startPos = file->_size;

    ssize_t desiredPos = startPos + offset;
    if (desiredPos < 0) return -2;
    if (desiredPos > (ssize_t)file->_size) return -3;

    file->_head = desiredPos;
    return 0;
  }

  /**
   * Indicates whether we've reached the end of file
   * 
   * @param[in] file The file to inquire for end of file
   * @return Non-zero, if the end has been reached. Zero, otherwise.
  */
  static __INLINE__ int feof(MemoryFile * const file) { return file->_head == file->_size; }

  /**
   * Sets the read only flag in the file
   */
  __INLINE__ void setReadOnly() { _readonly = true; }

  /**
   * Clears the read only flag from the file
   */
  __INLINE__ void unsetReadOnly() { _readonly = false; }

  /**
   * Sets the write only flag in the file
   */
  __INLINE__ void setWriteOnly() { _writeonly = true; }

  /**
   * Clears the read only flag from the file
   */
  __INLINE__ void unsetWriteOnly() { _writeonly = false; }

  /**
   * Sets the opened flag in the file
   */
  __INLINE__ void setOpened() { _opened = true; }

  /**
   * Clears the opened flag from the file
   */
  __INLINE__ void unsetOpened() { _opened = false; }

  
   /**
    * Gets the read-only flag from the file
    * @return The file's read-only flag
    */
  __INLINE__ bool isReadOnly() const { return _readonly; }

   /**
    * Gets the write-only flag from the file
    * @return The file's write-only flag
    */
  __INLINE__ bool isWriteOnly() const { return _writeonly; }

  /**
    * Gets the opened flag from the file
    * @return The file's opened flag
    */
  __INLINE__ bool isOpened() const { return _opened; }

  __INLINE__ void setWriteCallback(const std::function<void(const ssize_t, MemoryFile*)> callback) { _writeCallback = callback; _writeCallbackDefined = true; }
  __INLINE__ void setReadCallback(const std::function<void(const ssize_t, MemoryFile*)> callback)  { _readCallback  = callback; _readCallbackDefined  = true; }
  __INLINE__ void unsetWriteCallback() { _writeCallbackDefined = false; }
  __INLINE__ void unsetReadCallback() { _readCallbackDefined  = false; }

  private:

  /**
   * The file's buffer size
   */
  const size_t   _size;

  /**
   * The file's buffer
   */
  uint8_t *const _buffer;

  /**
   * The file's read-only flag
   */
  bool           _readonly  = false;

  /**
   * The file's write-only flag
   */
  bool           _writeonly = false;

  /**
   * The file's opened flag
   */
  bool           _opened    = false;

  /**
   * The file's internal head pointer
   */
  size_t         _head = 0;

  /**
   * Whether the write callback has been defined
   */
   bool _writeCallbackDefined = false;

  /**
   * The file's internal callback for writes
   */
   std::function<void(const ssize_t, MemoryFile* const)> _writeCallback;

   /**
   * Whether the write callback has been defined
   */
   bool _readCallbackDefined = false;

   /**
   * The file's internal callback for read
   */
   std::function<void(const ssize_t, MemoryFile* const)> _readCallback;
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
   * @param[in] size The buffer size for the file
   * 
   * @return The pointer to the new memory file, if successful. NULL, otherwise.
   */
  __INLINE__ MemoryFile *fopen(const std::string filename, const std::string mode, const size_t size = 0)
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

    // Checking if file already exists
    bool fileExists = _fileMap.contains(filename);

    // Evaluating the case where the file doesn't exist
    if (fileExists == false)
      {
        // If reading, return NULL
        if (openMode == mode_t::read && fileExists == false) return NULL;

        // Otherwise, create a new one
        _fileMap[filename] = std::make_unique<MemoryFile>(size);
    }

    // Evaluating the case where the file does exist
    if (fileExists == true)
    {
        // Check if opened. If it is, then we cannot re-open it now
        if (_fileMap.at(filename)->isOpened() == true) return NULL;

        // If reading, return NULL
        if (openMode == mode_t::read && fileExists == false) return NULL;

        // Otherwise, create a new one (a correct size needs to be provided)
        _fileMap[filename] = std::make_unique<MemoryFile>(size);
    }

    // Getting file
    MemoryFile *file = _fileMap.at(filename).get();

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
  int fclose(MemoryFile *const file)
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
  int fdestroy(const std::string filename)
  {
    // Checking if file already exists
    if (_fileMap.contains(filename) == false) return -1;

    // Getting file
    MemoryFile *file = _fileMap.at(filename).get();

    // Check if opened. Cannot delete it yet
    if (file->isOpened() == true) return -2;

    // Delete it now
    _fileMap.erase(filename);

    return 0;
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
