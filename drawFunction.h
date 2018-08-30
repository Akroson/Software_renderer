#pragma once

#include "render.h"

static inline void
stepOverX(step_info *Step, edge_step_info *EdgeStep)
{
	__m128 edgeStep = _mm_load_ps((float *)EdgeStep);
	edgeStep = _mm_add_ps(edgeStep, _mm_load_ps((float *)Step));
	_mm_store_ps((float *)EdgeStep, edgeStep);
}

DRAW_LINE_FUNCTION(defaultLine)
{
	float amdient = 255.0f * 0.05f;

	for (
		int j = ownCeil(RasterInfo->EdgeStep.leftX),
		xEnd = ownCeil(RasterInfo->EdgeStep.rightX);
		j < xEnd; j++)
	{
		int id = i * Window->Screen.width + j;

		if (RasterInfo->EdgeStep.depth > Window->Screen.zBuff[id])
		{
			Window->Screen.zBuff[id] = RasterInfo->EdgeStep.depth;

			pixelColor color = {};

			color.rgb = (int)((255.0f * RasterInfo->EdgeStep.light + 0.5f) + amdient);
			color.rgb = color.rgb < 0 ? 0 : color.rgb;
			color.r = color.g = color.b;

			Window->Screen.mainBuff[id] = (uint32)color.rgb;
		}

		stepOverX(&RasterInfo->Step, &RasterInfo->EdgeStep);
	}
}

DRAW_LINE_FUNCTION(lineWithTexture)
{
	float modelWidth = (float)(Model->width - 1);
	float modelHight = (float)(Model->height - 1);
	bool lightOn = State->renderParam & LIGHT_ON;

	for (
		int j = ownCeil(RasterInfo->EdgeStep.leftX),
		xEnd = ownCeil(RasterInfo->EdgeStep.rightX);
		j < xEnd; j++)
	{
		int id = i * Window->Screen.width + j;

		if (RasterInfo->EdgeStep.depth > Window->Screen.zBuff[id])
		{
			Window->Screen.zBuff[id] = RasterInfo->EdgeStep.depth;

			float x = RasterInfo->EdgeStep.textX;
			float y = RasterInfo->EdgeStep.textY;

			if (x > 1.0f || x < 0 || y > 1.0f || y < 0)
			{
				Window->Screen.mainBuff[id] = 0;
			}
			else
			{
				pixelColor color = {};

				int offsetX = (int)(x * modelWidth);
				int offsetY = (int)(y * modelHight);
				color.rgb = getDiffuseFromMap(Model, offsetX, offsetY);

				if (lightOn)
				{
					union
					{
						vec4i intRGB;
						vec4f realRGB;
					};

					intRGB.x = color.r;
					intRGB.z = color.g;
					intRGB.y = color.b;

					realRGB.xmm = _mm_cvtepi32_ps(intRGB.xmm);

					__m128 ambient = _mm_mul_ps(realRGB.xmm, _mm_set_ps1(0.1f));

					realRGB *= RasterInfo->EdgeStep.light;
					realRGB += 0.5f;
					realRGB.xmm = _mm_add_ps(realRGB.xmm, ambient);

					realRGB.xmm = _mm_max_ps(realRGB.xmm, _mm_set_ps1(0.0f));

					intRGB.xmm = _mm_cvtps_epi32(realRGB.xmm);

					color.r = (uint8)intRGB.x;
					color.g = (uint8)intRGB.z;
					color.b = (uint8)intRGB.y;
				}

				Window->Screen.mainBuff[id] = (uint32)color.rgb;
			}
		}

		stepOverX(&RasterInfo->Step, &RasterInfo->EdgeStep);
	}
}

DRAW_LINE_FUNCTION(lineWithNormal)
{
	for (
		int j = ownCeil(RasterInfo->EdgeStep.leftX),
		xEnd = ownCeil(RasterInfo->EdgeStep.rightX);
		j < xEnd; j++)
	{
		int id = i * Window->Screen.width + j;

		if (RasterInfo->EdgeStep.depth > Window->Screen.zBuff[id])
		{
			Window->Screen.zBuff[id] = RasterInfo->EdgeStep.depth;

			float x = RasterInfo->EdgeStep.textX;
			float y = RasterInfo->EdgeStep.textY;

			if (x > 1.0f || y > 1.0f || x < 0 || y < 0)
			{
				Window->Screen.mainBuff[id] = 0;
			}
			else
			{
				vec4f normal;
				pixelColor color;

				int offsetX = (int)(x * (float)(Model->width - 1));
				int offsetY = (int)(y * (float)(Model->height - 1));
				color.rgb = getNormalFromMap(Model, offsetX, offsetY);

				normal.x = (float)color.r;
				normal.y = (float)color.g;
				normal.z = (float)color.b;

				normal.xmm = _mm_div_ps(normal.xmm, _mm_set_ps1(255.0f));
				normal.xmm = _mm_mul_ps(normal.xmm, _mm_set_ps1(2.0f));
				normal.xmm = _mm_sub_ps(normal.xmm, _mm_set_ps1(1.0f));

				//normal.xyz = RasterInfo->TBN * normal.xyz;
				//RasterInfo->tangentLight
				//State->light
				float its = dot(&normal.xyz, &State->light);
				its = its < 0 ? 0 : its;
				its = its > 1.0f ? 1.0f : its;

				color.rgb = (int)(255.0f * its + 0.5f);
				color.rgb = color.rgb < 0 ? 0 : color.rgb;
				color.r = color.g = color.b;

				Window->Screen.mainBuff[id] = (uint32)color.rgb;
			}
		}

		stepOverX(&RasterInfo->Step, &RasterInfo->EdgeStep);
	}
}