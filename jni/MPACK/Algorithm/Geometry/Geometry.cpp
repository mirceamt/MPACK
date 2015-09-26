#include "Geometry.hpp"
#include "math.h"
#include "Geom.hpp"
#include <cassert>

using namespace std;
using namespace MPACK::Math;

namespace MPACK
{
	namespace Algorithm
	{
		struct ConvexHull_CMP
		{
			inline bool operator() (const Vector2f &A, const Vector2f &B)
			{
				return A.x < B.x || (A.x == B.x && A.y < B.y);
			}
		};

		float Cross(const Vector2f &O, const Vector2f &A, const Vector2f &B)
		{
			return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
		}

		vector<Vector2f> ConvexHull(vector<Vector2f> points)
		{
			int n = points.size(), k = 0;
			vector<Vector2f> hull(2*n);

			sort(points.begin(), points.end(), ConvexHull_CMP());

			for (int i = 0; i < n; i++)
			{
				while (k >= 2 && Cross(hull[k-2], hull[k-1], points[i]) <= 0)
				{
					k--;
				}
				hull[k++] = points[i];
			}

			for (int i = n-2, t = k+1; i >= 0; i--)
			{
				while (k >= t && Cross(hull[k-2], hull[k-1], points[i]) <= 0)
				{
					k--;
				}
				hull[k++] = points[i];
			}

			hull.resize(k);
			return hull;
		}

		std::vector<Math::Vector2f>& ClipPolygon(const std::vector<Math::Vector2f>& clip, std::vector<Math::Vector2f> polygon, std::vector<Math::Vector2f>& result)
		{
			// we assume that the points in the given clip polygon are already sorted in trigonometric order
			// we also assume that the given polygon to be clipped has vertices in a correct order
			// the given polygon to be clipped may not be necessary a convex one

			// verifying that we have a polygon
			int clipSize = clip.size();
			assert(clipSize >= 3);

			int turning = 0;
			auto moduloFunc = [](int x, int modValue) mutable -> int
					{
						while (x < 0) x += modValue;
						while (x >= modValue) x-= modValue;
						return x;
					};


			for (int i = 0; i < (int)clip.size(); ++ i)
			{
				int pozx, pozy, pozz;
				pozx = moduloFunc(i-1, clipSize);
				pozy = moduloFunc(i, clipSize);
				pozz = moduloFunc(i+1, clipSize);
				float det = Cross(clip[pozx], clip[pozy], clip[pozz]);

				assert(!(det < 0.f)); // pozx, pozy, pozz form a turning in reverse trigonometric order



				if (turning == 0)
				{
					if (det > 0.f)
					{
						turning = 1;
					}
				}
				else
				{
					assert(!(det < 0.f));
				}
			}

			assert(turning != 0);

			// now we are sure that the clipPolygon respects the constraints

			result = polygon;

			for (int i = 0; i < clipSize; ++ i)
			{
				// ( clip[pozx], clip[pozy] ) the line that cuts the polygon
				int pozx = moduloFunc(i-1, clipSize);
				int pozy = moduloFunc(i, clipSize);
				int polygonSize = polygon.size();

				int lastPoz = polygonSize - 1;

				polygon = result;
				result.clear();

				for (int j = 0; j < polygonSize; ++ j)
				{
					int signLast = 0, signNow = 0;
					float detLast = Cross(clip[pozx], clip[pozy], polygon[lastPoz]);
					float detNow = Cross(clip[pozx], clip[pozy], polygon[j]);

					if (detLast < 0.f) signLast = -1;
					if (detLast > 0.f) signLast = 1;

					if (detNow < 0.f) signNow = -1;
					if (detNow > 0.f) signNow = 1;

					// sign == 1 means that the vertex is on the correct side of the line

					if (signNow == 1)
					{
						if (signLast == -1)
						{
							Vector2f intersection;
							Geom<float>::LineIntersect(clip[pozx], clip[pozy], polygon[lastPoz], polygon[j], intersection);
							result.push_back(intersection);
						}
						else
						{
							result.push_back(polygon[lastPoz]);
						}
					}
					else if (signNow == 0)
					{
						if (signLast == 0 || signLast == 1)
						{
							result.push_back(polygon[lastPoz]);
						}
					}
					else
					{
						if (signLast == 0)
						{
							result.push_back(polygon[lastPoz]);
						}
						else if (signLast == 1)
						{
							Vector2f intersection;
							Geom<float>::LineIntersect(clip[pozx], clip[pozy], polygon[lastPoz], polygon[j], intersection);
							result.push_back(intersection);
						}
					}

					lastPoz = j;
				}
			}
			return result;
		}

		float PolygonArea (const std::vector <Math::Vector2f> & polygon)
		{
			float sum = 0.f;
			auto moduloFunc = [](int x, int modValue) mutable -> int
								{
									while (x < 0) x += modValue;
									while (x >= modValue) x-= modValue;
									return x;
								};
			for (int i = 0; i < (int)polygon.size(); ++ i)
			{
				sum += (polygon[i].x * polygon[moduloFunc(i+1, polygon.size())].y - polygon[moduloFunc(i+1, polygon.size())].x * polygon[i].y);
			}
			return sum;
		}
	}
}
