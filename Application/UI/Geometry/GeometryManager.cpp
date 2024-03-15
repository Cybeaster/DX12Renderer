#include "GeometryManager.h"

#include "GeometryEntity/PickedRenderItem.h"

#include <ranges>

void OGeometryManagerWidget::Draw()
{
	if (ImGui::CollapsingHeader("Geometry Manager"))
	{
		PickedRenderItemWidget->Draw();
		ImGui::Text("Number of geometries %d", RenderLayers->size());
		OGeometryEntityWidget* selectedWidget = nullptr;

		if (ImGui::TreeNode("Geometries"))
		{
			for (auto& entity : GetWidgets())
			{
				if (const auto widget = Cast<OGeometryEntityWidget>(entity.get()))
				{
					if (const auto geometry = widget->GetGeometry())
					{
						if (ImGui::Selectable(geometry->Name.c_str(), SelectedGeometry == geometry->Name))
						{
							SelectedGeometry = geometry->Name;
						}

						if (SelectedGeometry == geometry->Name)
						{
							selectedWidget = widget;
						}
					}
				}
			}
			ImGui::TreePop();
		}

		if (selectedWidget)
		{
			selectedWidget->Draw();
		}
	}
}

void OGeometryManagerWidget::InitWidget()
{
	IWidget::InitWidget();
	PickedRenderItemWidget = MakeWidget<OPickedRenderItemWidget>();
	for (auto& layer : *RenderLayers)
	{
		if (layer.first == "Opaque" || layer.first == "Transparent" || layer.first == "Sky" || layer.first == "Water")
		{
			for (auto item : layer.second)
			{
				if (item->Geometry)
				{
					MakeWidget<OGeometryEntityWidget>(item, Engine, this);
				}
			}
		}
	}
}

void OGeometryManagerWidget::RebuildRequest() const
{
	Engine->UpdateGeometryRequest(SelectedGeometry);
}