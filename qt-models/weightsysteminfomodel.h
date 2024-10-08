// SPDX-License-Identifier: GPL-2.0
#ifndef WEIGHTSYSTEMINFOMODEL_H
#define WEIGHTSYSTEMINFOMODEL_H

#include "cleanertablemodel.h"

/* Encapsulate ws_info */
class WSInfoModel : public CleanerTableModel {
	Q_OBJECT
public:
	enum Column {
		DESCRIPTION,
		GR
	};
	using CleanerTableModel::CleanerTableModel;

	QVariant data(const QModelIndex &index, int role) const override;
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
};

#endif
