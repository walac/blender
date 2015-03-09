/*
 * Copyright 2015, Blender Foundation.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <sstream>

#include <Alembic/AbcGeom/IGeomParam.h>
#include <Alembic/AbcGeom/OGeomParam.h>

#include "abc_customdata.h"

extern "C" {
#include "BLI_math.h"
#include "BLI_utildefines.h"

#include "DNA_customdata_types.h"
#include "DNA_meshdata_types.h"

#include "BKE_customdata.h"
}

namespace PTC {

using namespace Abc;
using namespace AbcGeom;

/* DEBUG */
BLI_INLINE void print_writer_compound(OCompoundProperty &prop)
{
	CompoundPropertyWriterPtr ptr = prop.getPtr()->asCompoundPtr();
	printf("compound %s: [%p] (%d)\n", ptr->getName().c_str(), ptr.get(), (int)ptr->getNumProperties());
	for (int i = 0; i < ptr->getNumProperties(); ++i) {
		printf("  %d: [%p]\n", i, prop.getProperty(i).getPtr().get());
		printf("      %s\n", prop.getProperty(i).getName().c_str());
	}
}

/* ========================================================================= */

template <CustomDataType CDTYPE>
static void write_sample(CustomDataWriter *writer, OCompoundProperty &parent, const std::string &name, void *data, int num_data)
{
	/* no implementation available, should not happen */
	printf("ERROR: CustomData type %s has no write_sample implementation\n", CustomData_layertype_name((int)CDTYPE));
}

template <>
void write_sample<CD_ORIGINDEX>(CustomDataWriter *writer, OCompoundProperty &parent, const std::string &name, void *data, int num_data)
{
	OInt32ArrayProperty prop = writer->add_array_property<OInt32ArrayProperty>(name, parent);
	
	prop.set(OInt32ArrayProperty::sample_type((int *)data, num_data));
}

template <>
void write_sample<CD_ORIGSPACE>(CustomDataWriter *writer, OCompoundProperty &parent, const std::string &name, void *data, int num_data)
{
	OCompoundProperty prop = writer->add_compound_property<OCompoundProperty>(name, parent);
	
	OV2fArrayProperty uv_prop[4];
	uv_prop[0] = writer->add_array_property<OV2fArrayProperty>("uv0", prop);
	uv_prop[1] = writer->add_array_property<OV2fArrayProperty>("uv1", prop);
	uv_prop[2] = writer->add_array_property<OV2fArrayProperty>("uv2", prop);
	uv_prop[3] = writer->add_array_property<OV2fArrayProperty>("uv3", prop);
	
	OrigSpaceFace *ospace = (OrigSpaceFace *)data;
	std::vector<V2f> uv_data[4];
	uv_data[0].reserve(num_data);
	uv_data[1].reserve(num_data);
	uv_data[2].reserve(num_data);
	uv_data[3].reserve(num_data);
	for (int i = 0; i < num_data; ++i) {
		uv_data[0].push_back(V2f(ospace[i].uv[0][0], ospace[i].uv[0][1]));
		uv_data[1].push_back(V2f(ospace[i].uv[1][0], ospace[i].uv[1][1]));
		uv_data[2].push_back(V2f(ospace[i].uv[2][0], ospace[i].uv[2][1]));
		uv_data[3].push_back(V2f(ospace[i].uv[3][0], ospace[i].uv[3][1]));
	}
	uv_prop[0].set(V2fArraySample(uv_data[0]));
	uv_prop[1].set(V2fArraySample(uv_data[1]));
	uv_prop[2].set(V2fArraySample(uv_data[2]));
	uv_prop[3].set(V2fArraySample(uv_data[3]));
}

/* ------------------------------------------------------------------------- */

template <CustomDataType CDTYPE>
static PTCReadSampleResult read_sample(CustomDataReader *reader, ICompoundProperty &parent, const ISampleSelector &ss, const std::string &name, void *data, int num_data)
{
	/* no implementation available, should not happen */
	printf("ERROR: CustomData type %s has no read_sample implementation\n", CustomData_layertype_name((int)CDTYPE));
	return PTC_READ_SAMPLE_INVALID;
}

template <>
PTCReadSampleResult read_sample<CD_ORIGINDEX>(CustomDataReader *reader, ICompoundProperty &parent, const ISampleSelector &ss, const std::string &name, void *data, int num_data)
{
	IInt32ArrayProperty prop = reader->add_array_property<IInt32ArrayProperty>(name, parent);
	
	Int32ArraySamplePtr sample = prop.getValue(ss);
	
	if (sample->size() != num_data)
		return PTC_READ_SAMPLE_INVALID;
	
	memcpy(data, sample->getData(), num_data);
	return PTC_READ_SAMPLE_EXACT;
}

template <>
PTCReadSampleResult read_sample<CD_ORIGSPACE>(CustomDataReader *reader, ICompoundProperty &parent, const ISampleSelector &ss, const std::string &name, void *data, int num_data)
{
	ICompoundProperty prop = reader->add_compound_property<ICompoundProperty>(name, parent);
	
	IV2fArrayProperty uv_prop[4];
	uv_prop[0] = reader->add_array_property<IV2fArrayProperty>("uv0", prop);
	uv_prop[1] = reader->add_array_property<IV2fArrayProperty>("uv1", prop);
	uv_prop[2] = reader->add_array_property<IV2fArrayProperty>("uv2", prop);
	uv_prop[3] = reader->add_array_property<IV2fArrayProperty>("uv3", prop);
	
	V2fArraySamplePtr sample0 = uv_prop[0].getValue(ss);
	V2fArraySamplePtr sample1 = uv_prop[1].getValue(ss);
	V2fArraySamplePtr sample2 = uv_prop[2].getValue(ss);
	V2fArraySamplePtr sample3 = uv_prop[3].getValue(ss);
	
	if (sample0->size() != num_data ||
	    sample1->size() != num_data ||
	    sample2->size() != num_data ||
	    sample3->size() != num_data)
		return PTC_READ_SAMPLE_INVALID;
	
	OrigSpaceFace *ospace = (OrigSpaceFace *)data;
	const V2f *data0 = (const V2f *)sample0->getData();
	const V2f *data1 = (const V2f *)sample1->getData();
	const V2f *data2 = (const V2f *)sample2->getData();
	const V2f *data3 = (const V2f *)sample3->getData();
	for (int i = 0; i < num_data; ++i) {
		copy_v2_v2(ospace->uv[0], data0->getValue());
		copy_v2_v2(ospace->uv[1], data1->getValue());
		copy_v2_v2(ospace->uv[2], data2->getValue());
		copy_v2_v2(ospace->uv[3], data3->getValue());
		
		++data0;
		++data1;
		++data2;
		++data3;
		++ospace;
	}
	
	return PTC_READ_SAMPLE_EXACT;
}

/* ========================================================================= */

/* recursive template that handles dispatch by CD layer type */
template <int CDTYPE>
BLI_INLINE void write_sample_call(CustomDataWriter *writer, OCompoundProperty &parent, CustomDataType type, const std::string &name, void *data, int num_data)
{
	if (type == CDTYPE)
		write_sample<(CustomDataType)CDTYPE>(writer, parent, name, data, num_data);
	else
		write_sample_call<CDTYPE+1>(writer, parent, type, name, data, num_data);
}

/* terminator specialization */
template <>
void write_sample_call<CD_NUMTYPES>(CustomDataWriter *writer, OCompoundProperty &parent, CustomDataType type, const std::string &name, void *data, int num_data)
{
}

/* ------------------------------------------------------------------------- */

/* recursive template that handles dispatch by CD layer type */
template <int CDTYPE>
BLI_INLINE PTCReadSampleResult read_sample_call(CustomDataReader *reader, ICompoundProperty &parent, const ISampleSelector &ss, CustomDataType type, const std::string &name, void *data, int num_data)
{
	if (type == CDTYPE)
		return read_sample<(CustomDataType)CDTYPE>(reader, parent, ss, name, data, num_data);
	else
		return read_sample_call<CDTYPE+1>(reader, parent, ss, type, name, data, num_data);
}

/* terminator specialization */
template <>
PTCReadSampleResult read_sample_call<CD_NUMTYPES>(CustomDataReader *reader, ICompoundProperty &parent, const ISampleSelector &ss, CustomDataType type, const std::string &name, void *data, int num_data)
{
	return PTC_READ_SAMPLE_INVALID;
}

/* ========================================================================= */

CustomDataWriter::CustomDataWriter(const std::string &name, CustomDataMask cdmask) :
    m_name(name),
    m_cdmask(cdmask)
{
}

CustomDataWriter::~CustomDataWriter()
{
	for (LayerPropsMap::iterator it = m_layer_props.begin(); it != m_layer_props.end(); ++it) {
		BasePropertyWriterPtr prop = it->second;
		if (prop)
			prop.reset();
	}
}

/* unique property name based on either layer name or index */
static std::string cdtype_to_name(CustomData *cdata, CustomDataType type, int n)
{
	const char *layer_name = CustomData_get_layer_name(cdata, type, n);
	std::string name;
	if (layer_name && layer_name[0] != '\0') {
		name = "S" + std::string(layer_name);
	}
	else {
		std::stringstream ss; ss << n;
		name = "N" + ss.str();
	}
	return name;
}

/* parse property name to CD layer name based on S or N prefix for named/unnamed layers */
static void cdtype_from_name(CustomData *cdata, const std::string &name, int *n, char *layer_name, int max_layer_name)
{
	if (name.empty()) {
		*n = -1;
		layer_name[0] = '\0';
	}
	else if (name[0] == 'S') {
		/* named layer */
		*n = -1;
		BLI_strncpy(layer_name, name.c_str() + 1, max_layer_name);
	}
	else if (name[0] == 'N') {
		/* unnamed layer */
		std::istringstream ss(name.c_str() + 1);
		ss >> (*n);
		layer_name[0] = '\0';
	}
	else {
		*n = -1;
		layer_name[0] = '\0';
	}
}

void CustomDataWriter::write_sample(CustomData *cdata, int num_data, OCompoundProperty &parent)
{
	/* compound property for all CD layers in the CustomData instance */
	m_props = add_compound_property<OCompoundProperty>(m_name, parent);
	
	for (int type = 0; type < CD_NUMTYPES; ++type) {
		CustomDataMask mask = (1 << type);
		/* only use specified types */
		if (!(mask & m_cdmask))
			continue;
		
		const char *layertype_name = CustomData_layertype_name(type);
		int num = CustomData_number_of_layers(cdata, type);
		
		OCompoundProperty layertype_props;
		for (int n = 0; n < num; ++n) {
			/* compound for all CD layers of the same type */
			if (!layertype_props)
				layertype_props = add_compound_property<OCompoundProperty>(layertype_name, m_props);
			
			std::string name = cdtype_to_name(cdata, (CustomDataType)type, n);
			void *data = CustomData_get_layer_n(cdata, type, n);
			write_sample_call<0>(this, layertype_props, (CustomDataType)type, name, data, num_data);
		}
	}
}

/* ------------------------------------------------------------------------- */

CustomDataReader::CustomDataReader(const std::string &name, CustomDataMask cdmask) :
    m_name(name),
    m_cdmask(cdmask)
{
}

CustomDataReader::~CustomDataReader()
{
	for (LayerPropsMap::iterator it = m_layer_props.begin(); it != m_layer_props.end(); ++it) {
		BasePropertyReaderPtr prop = it->second;
		if (prop)
			prop.reset();
	}
}

PTCReadSampleResult CustomDataReader::read_sample(const ISampleSelector &ss, CustomData *cdata, int num_data, ICompoundProperty &parent)
{
	m_props = add_compound_property<ICompoundProperty>(m_name, parent);
	
	for (int type = 0; type < CD_NUMTYPES; ++type) {
		CustomDataMask mask = (1 << type);
		/* only use specified types */
		if (!(mask & m_cdmask))
			continue;
		
		const char *layertype_name = CustomData_layertype_name(type);
		
		BasePropertyReaderPtr ptr = m_props.getPtr()->asCompoundPtr()->getProperty(layertype_name);
		if (!ptr) {
			/* no layer of this type stored */
			continue;
		}
		ICompoundProperty layertype_props(ptr->asCompoundPtr(), kWrapExisting);
		
		for (int i = 0; i < layertype_props.getNumProperties(); ++i) {
			const std::string &name = layertype_props.getPropertyHeader(i).getName();
			char layer_name[MAX_CUSTOMDATA_LAYER_NAME];
			int n;
			void *data;
			
			cdtype_from_name(cdata, name, &n, layer_name, sizeof(layer_name));
			if (layer_name[0] == '\0')
				data = CustomData_add_layer(cdata, type, CD_DEFAULT, NULL, num_data);
			else
				data = CustomData_add_layer_named(cdata, type, CD_DEFAULT, NULL, num_data, layer_name);
			
			read_sample_call<0>(this, layertype_props, ss, (CustomDataType)type, name, data, num_data);
		}
	}
	
	return PTC_READ_SAMPLE_EXACT;
}

} /* namespace PTC */