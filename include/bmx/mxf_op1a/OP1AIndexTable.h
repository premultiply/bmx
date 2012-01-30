/*
 * Copyright (C) 2011, British Broadcasting Corporation
 * All Rights Reserved.
 *
 * Author: Philip de Nier
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the British Broadcasting Corporation nor the names
 *       of its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __BMX_OP1A_INDEX_TABLE_H__
#define __BMX_OP1A_INDEX_TABLE_H__

#include <vector>

#include <bmx/ByteArray.h>



namespace bmx
{


class OP1AFile;


class OP1AIndexEntry
{
public:
    OP1AIndexEntry();
    OP1AIndexEntry(int8_t temporal_offset_, int8_t key_frame_offset_, uint8_t flags_, bool can_start_partition_);

    bool IsDefault();
    bool IsCompatible(const OP1AIndexEntry &entry);

public:
    int8_t temporal_offset;
    int8_t key_frame_offset;
    uint8_t flags;
    bool can_start_partition;
};


class OP1ADeltaEntry
{
public:
    OP1ADeltaEntry();

    int8_t pos_table_index;
    uint8_t slice;
    uint32_t element_delta;
};


class OP1AIndexTableElement
{
public:
    OP1AIndexTableElement(uint32_t track_index_, bool is_picture_, bool is_cbe_, bool apply_temporal_reordering_);

    void CacheIndexEntry(int64_t position, int8_t temporal_offset, int8_t key_frame_offset, uint8_t flags,
                         bool can_start_partition);
    void UpdateIndexEntry(int64_t position, int8_t temporal_offset);
    bool TakeIndexEntry(int64_t position, OP1AIndexEntry *entry);

    bool CanStartPartition(int64_t position);

public:
    uint32_t track_index;
    bool is_picture;
    bool is_cbe;
    bool apply_temporal_reordering;

    uint8_t slice_offset;

    uint32_t element_size;

private:
    std::map<int64_t, OP1AIndexEntry> mIndexEntryCache;
};


class OP1AIndexTableSegment
{
public:
    OP1AIndexTableSegment(uint32_t index_sid, uint32_t body_sid, mxfRational frame_rate, int64_t start_position,
                           uint32_t index_entry_size, uint32_t slice_count);
    ~OP1AIndexTableSegment();

    bool RequireNewSegment(uint8_t flags);
    void AddIndexEntry(const OP1AIndexEntry *entry, int64_t stream_offset, std::vector<uint32_t> slice_cp_offsets);
    void UpdateIndexEntry(int64_t segment_position, int8_t temporal_offset);

    void AddCBEIndexEntry(uint32_t edit_unit_byte_count);

    uint32_t GetDuration();

    mxfpp::IndexTableSegment* GetSegment() { return &mSegment; }
    ByteArray* GetEntries() { return &mEntries; }

private:
    mxfpp::IndexTableSegment mSegment;
    ByteArray mEntries;
    uint32_t mIndexEntrySize;
};


class OP1AIndexTable
{
public:
    OP1AIndexTable(uint32_t index_sid, uint32_t body_sid, mxfRational frame_rate);
    ~OP1AIndexTable();

    void RegisterPictureTrackElement(uint32_t track_index, bool is_cbe, bool apply_temporal_reordering);
    void RegisterAVCITrackElement(uint32_t track_index);
    void RegisterSoundTrackElement(uint32_t track_index);

    void PrepareWrite();

public:
    bool IsCBE() const { return mIsCBE; }
    bool IsVBE() const { return !mIsCBE; }
    bool RequireIndexTableSegmentPair() const { return mIsCBE && mHaveAVCI; }

    int64_t GetDuration() const { return mDuration; }
    int64_t GetStreamOffset() const { return mStreamOffset; }

public:
    void AddIndexEntry(uint32_t track_index, int64_t position, int8_t temporal_offset,
                       int8_t key_frame_offset, uint8_t flags, bool can_start_partition);
    void UpdateIndexEntry(uint32_t track_index, int64_t position, int8_t temporal_offset);

    bool CanStartPartition();

    void UpdateIndex(uint32_t size, std::vector<uint32_t> element_sizes);

public:
    bool HaveSegments();
    void WriteSegments(mxfpp::File *mxf_file, mxfpp::Partition *partition, bool final_write);

private:
    uint32_t mIndexSID;
    uint32_t mBodySID;
    mxfRational mFrameRate;

    std::vector<OP1AIndexTableElement*> mIndexElements;
    std::map<uint32_t, OP1AIndexTableElement*> mIndexElementsMap;
    bool mIsCBE;
    bool mHaveAVCI;
    uint8_t mSliceCount;
    uint32_t mIndexEntrySize;

    std::vector<OP1ADeltaEntry> mDeltaEntries;

    OP1AIndexTableSegment *mAVCIFirstIndexSegment;
    std::vector<OP1AIndexTableSegment*> mIndexSegments;
    int64_t mDuration;
    int64_t mStreamOffset;
};


};



#endif
