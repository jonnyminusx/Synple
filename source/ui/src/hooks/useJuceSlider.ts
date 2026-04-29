import { useEffect, useState, useCallback } from 'react'
import { getSliderState } from '../lib/juce/index.js'

type SliderProperties = ReturnType<typeof getSliderState>['properties']

const DEFAULT_PROPS: SliderProperties = {
  start: 0, end: 1, skew: 1, name: '', label: '', numSteps: 100, interval: 0, parameterIndex: -1,
}

export function useJuceSlider(id: string) {
  const [scaledValue, setScaledValue] = useState(0)
  const [properties, setProperties] = useState<SliderProperties>(DEFAULT_PROPS)

  useEffect(() => {
    const state = getSliderState(id)
    setScaledValue(state.getScaledValue())
    setProperties({ ...state.properties })

    const valueId = state.valueChangedEvent.addListener(() => setScaledValue(state.getScaledValue()))
    const propsId = state.propertiesChangedEvent.addListener(() => setProperties({ ...state.properties }))

    return () => {
      state.valueChangedEvent.removeListener(valueId)
      state.propertiesChangedEvent.removeListener(propsId)
    }
  }, [id])

  const normalisedValue = properties.end !== properties.start
    ? Math.pow(
        Math.max(0, Math.min(1, (scaledValue - properties.start) / (properties.end - properties.start))),
        properties.skew,
      )
    : 0

  const setNormalisedValue = useCallback(
    (v: number) => getSliderState(id).setNormalisedValue(v),
    [id],
  )
  const onDragStart = useCallback(() => getSliderState(id).sliderDragStarted(), [id])
  const onDragEnd = useCallback(() => getSliderState(id).sliderDragEnded(), [id])

  return { scaledValue, normalisedValue, properties, setNormalisedValue, onDragStart, onDragEnd }
}
