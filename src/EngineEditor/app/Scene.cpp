
#include <app/Scene.h>

namespace IHA {

	void Scene::AddObject(SceneObject* obj, SceneObject* parent)
	{
        auto newObj = std::unique_ptr<SceneObject>(obj);

        if (parent)
        {
            parent->children.push_back(newObj.get());
        }
        else
        {
            objects.push_back(std::move(newObj));
            return;
        }

        objects.push_back(std::move(newObj));
	}

	void Scene::RemoveObject(SceneObject* obj)
	{
        if (!obj) return;

        for (auto& o : objects)
        {
            // erase-remove idiom
            auto& children = o->children;
            children.erase(
                std::remove(children.begin(), children.end(), obj),
                children.end()
            );
        }

        objects.erase(
            std::remove_if(objects.begin(), objects.end(),
                [obj](const std::unique_ptr<SceneObject>& o) {
                    return o.get() == obj;
                }),
            objects.end()
        );
	}

	const std::vector<std::unique_ptr<SceneObject>>& Scene::GetObjects() const
	{
		return objects;
	}

}