/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <assert.h>
#include <stdlib.h>

#include <map>
#include <memory>

#include "common.h"
#include "dex_format.h"
#include "dex_ir.h"

class DexHelper;

namespace dex {

inline constexpr uint8_t opcode_len[] =
    {
        1, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 3, 2, 2,
        3, 5, 2, 2, 3, 2, 1, 1, 2, 2, 1, 2, 2, 3, 3, 3, 1, 1, 2, 3, 3, 3, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3,
        1, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2,  // removed in Android 12
        1, 1, 1, 1, 1, 1, 1, 4, 4, 3, 3, 2, 2};
static_assert(sizeof(opcode_len) == 256);

// Provides both a low level iteration over the .dex
// structures and incremental .dex IR creation.
//
// NOTES:
// - only little-endian .dex files and host machines are supported
// - aggresive structure validation & minimal semantic validation
//
class Reader {
 public:
  Reader(const dex::u1* image, size_t size, const dex::u1* data = nullptr,
         size_t data_size = 0);
  ~Reader() = default;

  Reader(Reader&&) = default;
  Reader& operator=(Reader&&) = default;

  // No copy/move semantics
  Reader(const Reader&) = delete;
  Reader& operator=(const Reader&) = delete;

 public:
  // Low level dex format interface
  const dex::Header* Header() const { return header_; }
  const bool IsCompact() const { return is_compact_; }
  const char* GetStringMUTF8(dex::u4 index) const;
  slicer::ArrayView<const dex::ClassDef> ClassDefs() const;
  slicer::ArrayView<const dex::StringId> StringIds() const;
  slicer::ArrayView<const dex::TypeId> TypeIds() const;
  slicer::ArrayView<const dex::FieldId> FieldIds() const;
  slicer::ArrayView<const dex::MethodId> MethodIds() const;
  slicer::ArrayView<const dex::ProtoId> ProtoIds() const;
  slicer::ArrayView<const dex::MethodHandle> MethodHandles() const;
  const dex::MapList* DexMapList() const;

  static bool IsCompact(const void* image) {
    const auto* header = reinterpret_cast<const struct Header*>(image);
    if (header->magic[0] == 'd' && header->magic[1] == 'e' &&
        header->magic[2] == 'x' && header->magic[3] == '\n') {
      return false;
    } else if (header->magic[0] == 'c' && header->magic[1] == 'd' &&
               header->magic[2] == 'e' && header->magic[3] == 'x') {
      return true;
    }
    return false;
  }

  // IR creation interface
  std::shared_ptr<ir::DexFile> GetIr() const { return dex_ir_; }
  void CreateFullIr();
  void CreateClassIr(dex::u4 index);
  dex::u4 FindClassIndex(const char* class_descriptor) const;

 private:
  // Internal access to IR nodes for indexed .dex structures
  ir::Class* GetClass(dex::u4 index);
  ir::Type* GetType(dex::u4 index);
  ir::FieldDecl* GetFieldDecl(dex::u4 index);
  ir::MethodDecl* GetMethodDecl(dex::u4 index);
  ir::Proto* GetProto(dex::u4 index);
  ir::String* GetString(dex::u4 index);
  ir::MethodHandle* GetMethodHandle(dex::u4 index);

  // Parsing annotations
  ir::AnnotationsDirectory* ExtractAnnotations(dex::u4 offset);
  ir::Annotation* ExtractAnnotationItem(dex::u4 offset);
  ir::AnnotationSet* ExtractAnnotationSet(dex::u4 offset);
  ir::AnnotationSetRefList* ExtractAnnotationSetRefList(dex::u4 offset);
  ir::FieldAnnotation* ParseFieldAnnotation(const dex::u1** pptr);
  ir::MethodAnnotation* ParseMethodAnnotation(const dex::u1** pptr);
  ir::ParamAnnotation* ParseParamAnnotation(const dex::u1** pptr);
  ir::EncodedField* ParseEncodedField(const dex::u1** pptr, dex::u4* baseIndex);
  ir::Annotation* ParseAnnotation(const dex::u1** pptr);
  ir::MethodHandle* ParseMethodHandle(dex::u4 index);

  // Parse encoded values and arrays
  ir::EncodedValue* ParseEncodedValue(const dex::u1** pptr);
  ir::EncodedArray* ParseEncodedArray(const dex::u1** pptr);
  ir::EncodedArray* ExtractEncodedArray(dex::u4 offset);

  // Parse root .dex structures
  ir::Class* ParseClass(dex::u4 index);
  ir::EncodedMethod* ParseEncodedMethod(const dex::u1** pptr,
                                        dex::u4* baseIndex);
  ir::Type* ParseType(dex::u4 index);
  ir::FieldDecl* ParseFieldDecl(dex::u4 index);
  ir::MethodDecl* ParseMethodDecl(dex::u4 index);
  ir::TypeList* ExtractTypeList(dex::u4 offset);
  ir::Proto* ParseProto(dex::u4 index);
  ir::String* ParseString(dex::u4 index);

  // Parse code and debug information
  ir::DebugInfo* ExtractDebugInfo(dex::u4 offset);
  ir::Code* ExtractCode(dex::u4 offset);
  void ParseInstructions(slicer::ArrayView<const dex::u2> code);

  // Convert a file pointer (absolute offset) to an in-memory pointer
  template <class T>
  const T* ptr(int offset) const {
    SLICER_CHECK_GE(offset, 0 && offset + sizeof(T) <= size_);
    return reinterpret_cast<const T*>(image_ + offset);
  }

  // Convert a data section file pointer (absolute offset) to an in-memory
  // pointer (offset should be inside the data section)
  template <class T>
  const T* dataPtr(size_t offset) const {
    SLICER_CHECK((is_compact_ || offset >= header_->data_off) &&
                 offset + sizeof(T) <= data_size_);
    return reinterpret_cast<const T*>(data_ + offset);
  }

  // Map an indexed section to an ArrayView<T>
  template <class T>
  slicer::ArrayView<const T> section(int offset, int count) const {
    return slicer::ArrayView<const T>(ptr<T>(offset), count);
  }

  // Simple accessor for a MUTF8 string data
  const dex::u1* GetStringData(dex::u4 index) const {
    auto& stringId = StringIds()[index];
    return dataPtr<dex::u1>(stringId.string_data_off);
  }

  void ValidateHeader();

 private:
  // the in-memory .dex image
  const dex::u1* image_;
  [[maybe_unused]] size_t size_;
  const dex::u1* data_;
  [[maybe_unused]] size_t data_size_;

  // .dex image header
  const dex::Header* header_;

  // .dex IR associated with the reader
  std::shared_ptr<ir::DexFile> dex_ir_;

  bool is_compact_;

  // maps for de-duplicating items identified by file pointers
  std::map<dex::u4, ir::TypeList*> type_lists_;
  std::map<dex::u4, ir::Annotation*> annotations_;
  std::map<dex::u4, ir::AnnotationSet*> annotation_sets_;
  std::map<dex::u4, ir::AnnotationsDirectory*> annotations_directories_;
  std::map<dex::u4, ir::EncodedArray*> encoded_arrays_;

  friend class ::DexHelper;
};

}  // namespace dex
