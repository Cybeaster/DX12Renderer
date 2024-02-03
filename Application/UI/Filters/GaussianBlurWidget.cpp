//
// Created by Cybea on 31/01/2024.
//

#include "GaussianBlurWidget.h"

#include "Filters/Blur/BlurFilter.h"
void OGaussianBlurWidget::Draw()
{
	bEnabled = ImGui::CollapsingHeader("Gaussian Blur");
	if (bEnabled)
	{
		ImGui::SliderFloat("Gaussian Sigma", &Sigma, 0.0f, 50.0f);
		ImGui::SliderInt("Gaussian Blur Count", &BlurCount, 1, 15);
	}
}

void OGaussianBlurWidget::Update()
{
	if (bEnabled)
	{
		Filter->SetParameters(BlurCount, Sigma);
	}
	else
	{
		Filter->SetParameters(1, 0);
	}
}