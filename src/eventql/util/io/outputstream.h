/**
 * Copyright (c) 2016 zScale Technology GmbH <legal@zscale.io>
 * Authors:
 *   - Paul Asmuth <paul@zscale.io>
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License ("the license") as
 * published by the Free Software Foundation, either version 3 of the License,
 * or any later version.
 *
 * In accordance with Section 7(e) of the license, the licensing of the Program
 * under the license does not imply a trademark license. Therefore any rights,
 * title and interest in our trademarks remain entirely with us.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You can be released from the requirements of the license by purchasing a
 * commercial license. Buying such a license is mandatory as soon as you develop
 * commercial activities involving this program without disclosing the source
 * code of your own applications
 */
#ifndef _libstx_OUTPUTSTREAM_H
#define _libstx_OUTPUTSTREAM_H
#include <fcntl.h>
#include <memory>
#include <mutex>
#include "eventql/util/buffer.h"
#include "eventql/util/io/file.h"

class OutputStream {
public:
  OutputStream() {}
  OutputStream(const OutputStream& other) = delete;
  OutputStream& operator=(const OutputStream& other) = delete;
  virtual ~OutputStream() {}

  /**
   * Get the stdout output stream
   */
  static std::unique_ptr<OutputStream> getStdout();

  /**
   * Get the stderr output stream
   */
  static std::unique_ptr<OutputStream> getStderr();

  /**
   * Write the next n bytes to the output stream. This may raise an exception.
   * Returns the number of bytes that have been written.
   *
   * @param data a pointer to the data to be written
   * @param size then number of bytes to be written
   */
  virtual size_t write(const char* data, size_t size) = 0;
  virtual size_t write(const std::string& data);
  virtual size_t write(const Buffer& buf);
  virtual size_t printf(const char* format, ...);

  /**
   * Writes a plain uint8 to the stream.
   */
  void appendUInt8(uint8_t value);

  /**
   * Writes a plain uint16 to the stream.
   */
  void appendUInt16(uint16_t value);

  /**
   * Writes a plain uint32 to the stream.
   */
  void appendUInt32(uint32_t value);

  /**
   * Writes a plain uint64 to the stream.
   */
  void appendUInt64(uint64_t value);

  /**
   * Writes a LEB128 encoded uint64 to the stream.
   */
  void appendVarUInt(uint64_t value);

  /**
   * Writes a IEEE754 encoded double to the stream.
   */
  void appendDouble(double value);

  /**
   * Writes a string to the stream.
   */
  void appendString(const std::string& string);

  /**
   * Writes a LEB128 prefix-length-encoded string to the stream.
   */
  void appendLenencString(const std::string& string);
  void appendLenencString(const void* data, size_t size);

  /**
   * Returns true if this output stream is connected to a TTY/terminal and false
   * otherwise
   */
  virtual bool isTTY() const;

  /**
   * Publicly accessible mutex that may be used to synchronize writes to this
   * output stream
   */
  mutable std::mutex mutex;

};

class FileOutputStream : public OutputStream {
public:

  /**
   * Create a new FileOutputStream instance by opening the provided file for
   * writing. The fille will automatically be close()ed when the output stream
   * is destroyed.
   *
   * @param file_path the path to the file to open
   * @param flags flags to pass to the open() syscall
   */
  static std::unique_ptr<FileOutputStream> openFile(
      const std::string& file_path,
      int flags = O_CREAT | O_TRUNC,
      int permissions = 0666);

  /**
   * Create a new FileOutputStream instance from the provided filedescriptor. If
   * close on_destroy is true, the fd will be close()ed when the input stream
   * is destroyed.
   *
   * @param fd a valid fd
   * @param close_on_destroy close the fd on destroy?
   */
  static std::unique_ptr<FileOutputStream> fromFileDescriptor(
      int fd,
      bool close_on_destroy = false);

  /**
   * Create a new FileOutputStream instance from the provided File.
   *
   * @param file the opened file
   */
  static std::unique_ptr<FileOutputStream> fromFile(File&& file);

  /**
   * Create a new FileOuputStream instance from the provided filedescriptor. If 
   * close on_destroy is true, the fd will be close()ed when the output stream
   * is destroyed.
   *
   * @param fd a valid fd
   * @param close_on_destroy close the fd on destroy?
   */
  explicit FileOutputStream(int fd, bool close_on_destroy = false);

  /**
   * Create a new FileOutputStream instance from the provided File.
   *
   * @param file the opened file
   */
  explicit FileOutputStream(File&& file);

  /**
   * Close the fd if close_on_destroy is true
   */
  ~FileOutputStream();

  /**
   * Write the next n bytes to the file. This may raise an exception.
   * Returns the number of bytes that have been written.
   *
   * @param data a pointer to the data to be written
   * @param size then number of bytes to be written
   */
  size_t write(const char* data, size_t size) override;

  size_t printf(const char* format, ...) override;

  /**
   * Returns true if this file descriptor is connected to a TTY/terminal and
   * false otherwise
   */
  bool isTTY() const override;

  /**
   * Seek to the provided offset in number of bytes from the beginning of the
   * file
   */
  void seekTo(size_t offset);

protected:
  int fd_;
  bool close_on_destroy_;
};

class StringOutputStream : public OutputStream {
public:

  /**
   * Create a new OutputStream from the provided string
   *
   * @param string the input string
   */
  static std::unique_ptr<StringOutputStream> fromString(std::string* string);

  /**
   * Create a new OutputStream from the provided string
   *
   * @param string the input string
   */
  StringOutputStream(std::string* string);

  /**
   * Write the next n bytes to the file. This may raise an exception.
   * Returns the number of bytes that have been written.
   *
   * @param data a pointer to the data to be written
   * @param size then number of bytes to be written
   */
  size_t write(const char* data, size_t size) override;

protected:
  std::string* str_;
};

class BufferOutputStream : public OutputStream {
public:

  /**
   * Create a new OutputStream from the provided string
   *
   * @param buf the output buffer
   */
  static std::unique_ptr<BufferOutputStream> fromBuffer(Buffer* buf);

  /**
   * Create a new OutputStream from the provided buffer
   *
   * @param buf the output buffer
   */
  BufferOutputStream(Buffer* string);

  /**
   * Write the next n bytes to the file. This may raise an exception.
   * Returns the number of bytes that have been written.
   *
   * @param data a pointer to the data to be written
   * @param size then number of bytes to be written
   */
  size_t write(const char* data, size_t size) override;

protected:
  Buffer* buf_;
};

#endif
